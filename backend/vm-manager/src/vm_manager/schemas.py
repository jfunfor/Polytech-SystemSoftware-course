from pydantic import BaseModel

class VMCreateRequest(BaseModel):
    name: str
    cpu_cores: int = 2
    memory_gb: int = 2
    disk_size_gb: int = 20

class VMCreateResponse(BaseModel):
    id: str
    status: str
    ssh_details: dict

class VMDestroyRequest(BaseModel):
    vm_id: str

class VMDestroyResponse(BaseModel):
    success: bool