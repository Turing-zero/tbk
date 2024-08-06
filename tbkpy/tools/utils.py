import os
def etcdadm_bin():
    HOMEDIR = os.path.expanduser("~")
    return os.path.join(HOMEDIR,'.tbk/etcdadm/bin/etcdadm')

def etcdctl_bin():
    HOMEDIR = os.path.expanduser("~")
    return os.path.join(HOMEDIR,'.tbk/etcdadm/bin/etcdctl.sh')
