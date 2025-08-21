from pydantic import BaseModel
from enum import Enum

class VMStatus(str, Enum):
    RUNNING = "running"
    DELETED_BY_REQUEST = "deleted by request"
    DELETED_BY_TIMER = "deleted by timer"

class VMCreateRequest(BaseModel):
    task_id: str
    cpu_cores: int = 2
    memory_gb: int = 2
    disk_size_gb: int = 20

class VMCreateResponse(BaseModel):
    vm_id: str
    ttyd_url: str
    ttyd_user: str
    ttyd_password: str


class VMGetRequest(BaseModel):
    vm_id: str


class VMGetResponse(BaseModel):
    vm_id: str
    task_id: str
    time_start: str
    time_end: str
    status: VMStatus


class CheckTaskRequest(BaseModel):
    vm_id: str


class CheckTaskResponse(BaseModel):
    task_id: str
    grade: int
    feedback: list
    
class VMDestroyRequest(BaseModel):
    vm_id: str

class VMDestroyResponse(BaseModel):
    success: bool