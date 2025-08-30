from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Column, String, Integer, DateTime
from sqlalchemy.sql import func  
from datetime import datetime 

Base = declarative_base()

class VM(Base):
    __tablename__ = "vms"
    id = Column(String, primary_key=True, index=True)
    name = Column(String, nullable=False)
    cpu_cores = Column(Integer, nullable=False)
    memory_gb = Column(Integer, nullable=False)
    ssh_user = Column(String, nullable=False)
    ssh_password = Column(String, nullable=False)
    ip_addres = Column(String, default='0.0.0.0', nullable=False)
    created_at = Column(DateTime(timezone=True), server_default=func.now(), nullable=False)
    task_id = Column(String, nullable=False)
    deleted_at = Column(DateTime(timezone=True), nullable=True)
    status = Column(String, nullable=True)
