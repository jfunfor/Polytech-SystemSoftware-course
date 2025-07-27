from fastapi import APIRouter
from fastapi import Depends
from fastapi import HTTPException
from ..schemas import (
    VMCreateRequest,
    VMCreateResponse,
    VMDestroyRequest,
    VMDestroyResponse
)
from ..services.terraform_manager import TerraformManager
from ..db import get_db
from ..models import VM

router = APIRouter()
tf_manager = TerraformManager()

@router.post("/vms/", response_model=VMCreateResponse)
def create_vm(request: VMCreateRequest, db = Depends(get_db)):
    result = tf_manager.create_vm(
        vm_name=request.name,
        cpu_cores=request.cpu_cores,
        memory_gb=request.memory_gb
    )   

    vm = VM(
        id=str(result["vm_id"]),
        name=request.name,
        cpu_cores=request.cpu_cores,
        memory_gb=request.memory_gb,
        ssh_user=result["ssh_user"],
        ssh_password=result["ssh_password"],
        ip_addres=result["ip_addres"]
    )

    db.add(vm)
    db.commit()
    
    return VMCreateResponse(
        id = result["vm_id"],
        status='created',
        ssh_details={
            "user": result["ssh_user"],
            "password": result["ssh_password"],
            "ip_addres": result["ip_addres"]
        }
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

    db.delete(vm)
    db.commit()

    return(VMDestroyResponse(success=True))