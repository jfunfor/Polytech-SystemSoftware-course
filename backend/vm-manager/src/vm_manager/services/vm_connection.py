import paramiko

class RemoteDeployer:
    def __init__(self, host, username, password, port=22):
        self.host = host
        self.username = username
        self.password = password
        self.port = port
        self.ssh = paramiko.SSHClient()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.ssh.connect(self.host, username=self.username, password=self.password, port=self.port)

    def execute_command(self, command):
        stdin, stdout, stderr = self.ssh.exec_command(command)
        output = stdout.read().decode('utf-8')
        error = stderr.read().decode('utf-8')
        return output, error
    
    def add_ed25519_private_key(self, ssh_key=None, remote_path=None):

        if not ssh_key:
            raise ValueError(f'Переменная окружения не установлена или пуста.')

        if remote_path is None:
            remote_path = f'/home/{self.username}/.ssh/id_ed25519'

        commands = [
            f'mkdir -p /home/{self.username}/.ssh',
            f'echo "{ssh_key.strip()}" > {remote_path}',
            f'chmod 600 {remote_path}',
            f'chown {self.username}:{self.username} {remote_path}'
        ]
        for cmd in commands:
            self.execute_command(cmd)
        return True
    
    
    def install_build_essential(self):
        #cmd = "sudo DEBIAN_FRONTEND=noninteractive apt-get update && sudo DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential"
        cmd = 'sudo DEBIAN_FRONTEND=noninteractive apt upgrade -y --fix-missing ; sudo DEBIAN_FRONTEND=noninteractive apt install -y build-essential'
        return self.execute_command(cmd)

    def clone_recipe_repo(self, repo_url, dest_dir=None):
        if dest_dir is not None:
            cmd = f"GIT_SSH_COMMAND='ssh -o StrictHostKeyChecking=no' git clone {repo_url} {dest_dir}"
        else:
            cmd = f"GIT_SSH_COMMAND='ssh -o StrictHostKeyChecking=no' git clone {repo_url}"
        return self.execute_command(cmd)
    
    def delete_recipe_repo(self, dir):
        cmd = f'rm -rf {dir}'
        return self.execute_command(cmd)
    
    def build_executable(self, build_command, work_dir=None):
        if work_dir is not None:
            cmd = f"cd {work_dir} && {build_command}"
        else:
            cmd = build_command
        return self.execute_command(cmd)
    
    def run_executable(self, exec_path, args='', work_dir=None):
        cmd_line = f"./{exec_path} {args}".strip()
        if work_dir is not None:
            cmd = f"cd {work_dir} && {cmd_line}"
        else:
            cmd = cmd_line
        return self.execute_command(cmd)
    
    def run_ttyd(self, port=7681, auth_user=None, auth_pwd=None, command='bash'):
        stop_ttyd_daemon_cmd = 'sudo systemctl stop ttyd'
        self.execute_command(stop_ttyd_daemon_cmd)
        auth = ""
        if auth_user is not None and auth_pwd is not None:
            auth = f"--credential {auth_user}:{auth_pwd}"
        cmd = f"nohup ttyd {auth} -p {port} {command} > /dev/null 2>&1 &"
        return self.execute_command(cmd)

    def close(self):
        self.ssh.close()