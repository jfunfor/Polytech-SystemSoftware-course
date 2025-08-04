from fastapi import FastAPI
from .api.endpoints import router as vm_router

app = FastAPI(
    title="VM Manager API",
    description="Управление виртуальными машинами в Proxmox"
)

app.include_router(vm_router)
