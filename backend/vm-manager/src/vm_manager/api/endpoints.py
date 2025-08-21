from fastapi import APIRouter
from fastapi import Depends
from fastapi import HTTPException
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
from ..db import get_db
from ..models import VM

from dotenv import load_dotenv
import os
from pathlib import Path

from src.vm_manager.models import Base

load_dotenv(Path(__file__).parent.parent / '.env')

router = APIRouter()
tf_manager = TerraformManager()

@router.post("/vms/", response_model=VMCreateResponse)
def create_vm(request: VMCreateRequest, db = Depends(get_db)):
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

    conn = RemoteDeployer(
        vm.ip_addres,
        vm.ssh_user,
        vm.ssh_password
    )

    conn.add_ed25519_private_key(os.getenv("SSH_PRIVATE_KEY"))

    conn.install_build_essential()
    conn.clone_recipe_repo(os.getenv("RECIPE_REPO_URL"))
    conn.build_executable('make', 'Polytech-SystemSoftware-course/backend/recp')
    conn.run_executable('recp', work_dir='Polytech-SystemSoftware-course/backend/recp')
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
def destroy_vm(vm_id: str, db=Depends(get_db)):
    
    vm = db.get(VM, vm_id)
    if not vm:
        raise HTTPException(status_code=404, detail="VM не найдена")
    
    try:
        tf_manager.delete_vm(vm_id)
    except Exception as e:
        return(VMDestroyResponse(success=False))

    vm.status = VMStatus.DELETED_BY_REQUEST
    db.commit()

    return(VMDestroyResponse(success=True))

@router.post("/vms/check_task/", response_model=CheckTaskResponse)
def check_task(request: CheckTaskRequest, db = Depends(get_db)):
    vm = db.get(VM, request.vm_id)
    if not vm:
        raise HTTPException(status_code=404, detail="VM не найдена") 


    conn = RemoteDeployer(
        vm.ip_addres,
        vm.ssh_user,
        vm.ssh_password
    )

    conn.clone_recipe_repo(os.getenv("RECIPE_REPO_URL"))
    conn.build_executable('make', work_dir='Polytech-SystemSoftware-course/backend/lchk')
    output, err = conn.run_executable('lchk', work_dir='Polytech-SystemSoftware-course/backend/lchk')
    conn.delete_recipe_repo('Polytech-SystemSoftware-course/')

    conn.close()

    try:
        data = json.load(output)
    except Exception as e:
        print(output, flush=True)
        print(err, flush=True)
        raise HTTPException(status_code=500, detail="Ошибка чтения результатов автопроверки") 

    return CheckTaskResponse(
        task_id=data['task_id'],
        grade=data['grade'],
        feedback=data.get('feedback', [])
    )