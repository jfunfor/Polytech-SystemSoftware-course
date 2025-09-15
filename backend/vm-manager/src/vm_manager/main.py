from fastapi import FastAPI
from fastapi_keycloak_middleware import KeycloakConfiguration, setup_keycloak_middleware
from dotenv import load_dotenv
import os
from pathlib import Path
from fastapi.middleware.cors import CORSMiddleware
from .api.endpoints import router, lifespan

if os.getenv("DOCKER_ENV") != "true":
    load_dotenv(Path(__file__).parent.parent / '.env')

keycloak_config = KeycloakConfiguration(
    url=os.getenv("KEYCLOAK_URL"),
    realm=os.getenv("KEYCLOAK_REALM"),
    client_id=os.getenv("KEYCLOAK_VM_MANAGER_ID"),
    client_secret=os.getenv("KEYCLOAK_VM_MANAGER_SECRET"),
    reject_on_missing_claim=False,
)

app = FastAPI(lifespan=lifespan)

app.include_router(router)

setup_keycloak_middleware(
    app,
    keycloak_configuration=keycloak_config,
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)
