import subprocess
from pathlib import Path
import secrets
import yaml
import uuid
import shutil
import  json
from dotenv import load_dotenv
import os
from jinja2 import Template
from python_terraform import Terraform

if os.getenv("DOCKER_ENV") != "true":
    load_dotenv(Path(__file__).parent.parent / '.env')

class TerraformManager:
    def __init__(self):
            self.terraform = Terraform()

    def generate_cloud_init(self, vm_dir: Path, ssh_user: str, ssh_password: str):
        with open("terraform/templates/cloud-init.j2") as f:
            cloud_init = Template(f.read()).render(
                ssh_user=ssh_user,
                ssh_password=ssh_password
            )
        (vm_dir / "cloud-init.yml").write_text(cloud_init)

    def create_vm(self, cpu_cores: int, memory_gb: int, disk_size: int):
        vm_id = str(uuid.uuid4())
        vm_name = str(vm_id).replace("-", "")
        vm_dir = Path(f"./terraform/generated/{vm_id}")
        vm_dir.mkdir(parents=True, exist_ok=True)

        ssh_password = secrets.token_urlsafe(12)
        for file in ["variables.tf", "provider.tf", "outputs.tf", "main.tf"]:
            shutil.copy(f"./terraform/{file}", vm_dir)

        secrets_content = f"""
            proxmox_api_token_secret = "{os.getenv('PROXMOX_API_TOKEN_SECRET')}"
            proxmox_api_token_id = "{os.getenv('PROXMOX_API_TOKEN_ID')}"
            proxmox_api_url = "{os.getenv('PROXMOX_API_URL')}"
        """
        print(secrets_content, flush=True)
        (vm_dir / "secrets.auto.tfvars").write_text(secrets_content)
        
        tfvars_content = f"""
            vm_name = "{vm_name}"
            cpu_cores = {cpu_cores}
            memory_gb = {memory_gb * 1024}
            disk_size_gb = {disk_size}
            vm_user = "student"
            vm_password = "{ssh_password}"
            """
        (vm_dir / "terraform.tfvars").write_text(tfvars_content.strip())

        try:
            init_result = subprocess.run(
                ["terraform", "init"],
                capture_output=True, text=True, cwd=str(vm_dir),
            )
            if init_result.returncode != 0:
                raise Exception(f"Terraform init failed: {init_result.stderr}")
            
            apply_result = subprocess.run(
                ["terraform", "apply", "-auto-approve"],
                capture_output=True, text=True, cwd=str(vm_dir),
            )
            if apply_result.returncode != 0:
                raise Exception(f"Terraform init failed: {apply_result.stderr}")
            
            output_result = subprocess.run(
                ["terraform", "output", "-json"],
                capture_output=True, text=True, cwd=str(vm_dir)
            )
            if output_result.returncode != 0:
                raise Exception(f"Failed to get outputs: {output_result.stderr}")
            
            outputs = json.loads(output_result.stdout)
            ip_addres = outputs['vm_ip_address']['value']
        except Exception as e:
            print(f"Error: {str(e)}")
            raise
        finally:
            secrets_file = vm_dir / "secrets.auto.tfvars"
            if secrets_file.exists():
                secrets_file.unlink()

        return {
            "vm_id": vm_id,
            "vm_name": vm_name,
            "ssh_password": ssh_password,
            "ssh_user": "student",
            "ip_addres": ip_addres
        }
    
    def delete_vm(self, vm_id: str):
        vm_dir = Path(f"./terraform/generated/{vm_id}")

        if not vm_dir.exists():
            raise Exception(f"Каталог для VM с id {vm_id} не найден.")

        try:
            destroy_result = subprocess.run(
                ["terraform", "destroy", "-auto-approve"],
                capture_output=True, text=True, cwd=str(vm_dir)
            )
            if destroy_result.returncode != 0:
                raise Exception(f"Terraform destroy failed: {destroy_result.stderr}")

            shutil.rmtree(vm_dir)
        except Exception as e:
            print(f"Error during VM deletion: {e}")
            raise