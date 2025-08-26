from fastapi import FastAPI
from .api.endpoints import router, lifespan

app = FastAPI(lifespan=lifespan)
app.include_router(router)
