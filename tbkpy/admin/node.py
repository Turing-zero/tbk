import sys
# add the path of tbkpy to sys.path
sys.path.append("..")
import os
import paramiko
import subprocess
import json

class Remote:
    def __init__(self, host, user, password):
        self.host = host
        self.user = user
        self.password = password
        self.client = paramiko.SSHClient()
        self.client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.client.connect(host, username=user, password=password)

    def execute(self, command):
        stdin, stdout, stderr = self.client.exec_command(command)
        return stdout.read().decode()

    def getFile(self, remotePath, localPath):
        sftp = self.client.open_sftp()
        sftp.get(remotePath, localPath)
        sftp.close()
    
    def putFile(self, localPath, remotePath):
        sftp = self.client.open_sftp()
        sftp.put(localPath, remotePath)
        sftp.close()

    def checkExists(self, file):
        return self.execute(f"test -f {file} && echo 1 || echo 0").strip() == "1"

    def close(self):
        self.client.close()
class TBKNode:
    def __init__(self, host, user, password):
        self.remote = Remote(host, user, password)
        self.remote_home = self.remote.execute("echo $HOME").strip()

        self.etcdadm_bin = f"{self.remote_home}/.tbk/etcdadm/bin/etcdadm"
        self.etcd_bin = f"{self.remote_home}/.tbk/etcdadm/bin/etcd"
        self.etcdctl_bin = f"{self.remote_home}/.tbk/etcdadm/bin/etcdctl.sh"

        print(f"status : {self.__getRemoteStatus()}")
    def reset(self):
        self.remote.execute(f"{self.etcdadm_bin} reset")
        status, output = self.__getRemoteStatus()
        return status, output.strip()
    def join(self):
        # step 1 - copy local CA
        self.__copyLocalCA()
        # step2 - get local etcd endpoint
        localClientUrl = self.__getLocalEndpoint()
        if localClientUrl is None:
            print("Error: could not get local etcd endpoint")
            return
        # step 3 - join
        self.remote.execute(f"{self.etcdadm_bin} join {localClientUrl}")
        # step 4 - check status
        status, output = self.__getRemoteStatus()
        return status, output.strip()
    
    def __getRemoteStatus(self):
        installed_etcdadm = self.remote.checkExists(self.etcdadm_bin)
        installed_etcd = self.remote.checkExists(self.etcd_bin)
        installed_etcdctl = self.remote.checkExists(self.etcdctl_bin)
        if installed_etcdctl:
            output = self.remote.execute(f"{self.etcdctl_bin} endpoint status --cluster --command-timeout=1s")
            return True, output
        return False, "etcdctl not installed"
    def __copyLocalCA(self):
        remote_path = f"{self.remote_home}/.tbk/etcdadm/pki"
        self.remote.execute(f"mkdir -p {remote_path}")

        files,local_path = self.__getLocalCA()
        for file in files:
            self.remote.putFile(os.path.join(local_path,file), os.path.join(remote_path,file))

    def __getLocalCA(self):
        # ls $HOME/.tbk/etcdadm/pki/ca.*
        dir = os.path.join(os.path.expanduser("~"), ".tbk/etcdadm/pki")
        if not os.path.exists(dir):
            return []
        files = [filename for filename in os.listdir(dir) if filename.startswith("ca.")]
        return files, dir
    
    def __getLocalEndpoint(self):
        localEtcdadmPath = os.path.join(os.path.expanduser("~"), ".tbk/etcdadm/bin/etcdadm")
        cmd = subprocess.run([localEtcdadmPath, "info"], stdout=subprocess.PIPE)
        # check if error
        if cmd.returncode != 0:
            print("Error: ", cmd.stdout.decode())
            return None
        # parse output
        output = json.loads(cmd.stdout.decode())
        if 'clientURLs' not in output:
            print("Error: clientURLs not found in output")
            return None
        return output['clientURLs'][0]

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="tool to manage tbk slave node")
    parser.add_argument("host", help="host of the slave node")
    parser.add_argument("user", help="user of the slave node")
    parser.add_argument("password", help="password of the slave node")
    commands = parser.add_subparsers(dest="command", help="action")
    join = commands.add_parser("join", help="set slave node join the cluster")
    reset = commands.add_parser("reset", help="reset the slave node")
    args = parser.parse_args()
    node = TBKNode(args.host, args.user, args.password)
    if args.command == "join":
        print(f"join  : {node.join()}")
    elif args.command == "reset":
        print(f"reset : {node.reset()}")
    else:
        print(f"Error: command {args.command} not found")
