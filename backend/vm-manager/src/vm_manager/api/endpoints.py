from fastapi import (
    FastAPI,
    APIRouter,
    Depends,
    HTTPException,
    BackgroundTasks,
    status
)
from sqlalchemy import func
import asyncio
import contextlib
import json
from ..schemas import (
    VMCreateRequest,
    VMCreateResponse,
    VMDestroyRequest,
    VMDestroyResponse,
    VMStatus,
    CheckTaskRequest,
    CheckTaskResponse
)
from ..services.terraform_manager import TerraformManager
from ..services.vm_connection import RemoteDeployer
from ..services.vm_timer import(
    set_redis_ttl_for_vm,
    vm_cleanup_worker,
    delete_redis_key,
    extend_redis_ttl
)
from ..db import get_db
from ..models import VM

from dotenv import load_dotenv
import os
from pathlib import Path

from src.vm_manager.models import Base

if os.getenv("DOCKER_ENV") != "true":
    load_dotenv(Path(__file__).parent.parent / '.env')

router = APIRouter()
tf_manager = TerraformManager()


@router.post("/vms/", response_model=VMCreateResponse)
async def create_vm(
    request: VMCreateRequest,
    background_tasks: BackgroundTasks,
    db = Depends(get_db),
):
    running_vms_count = db.query(func.count(VM.id)).filter(VM.status == VMStatus.RUNNING).scalar()

    if running_vms_count >= 10:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Превышено максимальное количество активных VM (10). Попробуйте позже."
        )

    result = tf_manager.create_vm(
        cpu_cores=request.cpu_cores,
        memory_gb=request.memory_gb,
        disk_size=request.disk_size_gb
    )   

    vm = VM(
        id=str(result["vm_id"]),
        name=result["vm_name"],
        cpu_cores=request.cpu_cores,
        memory_gb=request.memory_gb,
        ssh_user=result["ssh_user"],
        ssh_password=result["ssh_password"],
        ip_addres=result["ip_addres"],
        status=VMStatus.RUNNING,
        task_id=request.task_id
    )

    db.add(vm)
    db.commit()

    background_tasks.add_task(set_redis_ttl_for_vm, vm.id)

    conn = RemoteDeployer(
        vm.ip_addres,
        vm.ssh_user,
        vm.ssh_password
    )

    conn.add_ed25519_private_key(os.getenv("SSH_PRIVATE_KEY"))

    #conn.install_build_essential()
    conn.clone_recipe_repo(os.getenv("RECIPE_REPO_URL"))
    conn.build_executable('make', 'Polytech-SystemSoftware-course/backend/recp')
    conn.run_executable(
        'recp',
        args=f'-r ~/Polytech-SystemSoftware-course/backend/assets/recipes/{vm.task_id}.recipe',
        work_dir='Polytech-SystemSoftware-course/backend/recp'
    )
    conn.delete_recipe_repo('Polytech-SystemSoftware-course/')


    conn.run_ttyd(auth_user=vm.ssh_user, auth_pwd=vm.ssh_password)

    conn.close()

    ttyd_url = f"http://{vm.ip_addres}:{7681}"
    return VMCreateResponse(
        vm_id=vm.id,
        ttyd_url=ttyd_url,
        ttyd_user=vm.ssh_user,
        ttyd_password=vm.ssh_password
    )
    

@router.delete("/vms/{vm_id}/", response_model=VMDestroyResponse)
async def destroy_vm(vm_id: str, db=Depends(get_db)):
    
    vm = db.get(VM, vm_id)
    if not vm:
        raise HTTPException(status_code=404, detail="VM не найдена")
    
    try:
        tf_manager.delete_vm(vm_id)
        await delete_redis_key(vm_id)
    except Exception as e:
        return(VMDestroyResponse(success=False))

    vm.status = VMStatus.DELETED_BY_REQUEST
    db.commit()

    return(VMDestroyResponse(success=True))

@router.post("/vms/check_task/", response_model=CheckTaskResponse)
async def check_task(request: CheckTaskRequest, db = Depends(get_db)):
    vm = db.get(VM, request.vm_id)
    if not vm:
        raise HTTPException(status_code=404, detail="VM не найдена") 

    if vm.status != VMStatus.RUNNING:
        raise HTTPException(status_code=400, detail="VM отключена") 

    await extend_redis_ttl(request.vm_id)

    conn = RemoteDeployer(
        vm.ip_addres,
        vm.ssh_user,
        vm.ssh_password
    )

    conn.clone_recipe_repo(os.getenv("RECIPE_REPO_URL"))
    conn.build_executable('make', work_dir='Polytech-SystemSoftware-course/backend/lchk')
    output, err = conn.run_executable(
        'lchk',
        args=f'-t {vm.task_id} -f',
        work_dir='Polytech-SystemSoftware-course/backend/lchk'
    )
    conn.delete_recipe_repo('Polytech-SystemSoftware-course/')

    conn.close()

    try:
        data = json.loads(output)
    except Exception as e:
        print(output)
        #print(err, flush=True)
        raise HTTPException(status_code=500, detail="Ошибка чтения результатов автопроверки") 

    return CheckTaskResponse(
        task_id=data['task_id'],
        grade=data['grade'],
        feedback=data.get('feedback', [])
    )

@contextlib.asynccontextmanager
async def lifespan(app: FastAPI):
    cleanup_task = asyncio.create_task(vm_cleanup_worker(tf_manager))
    yield
    cleanup_task.cancel()
    with contextlib.suppress(asyncio.CancelledError):
        await cleanup_task
