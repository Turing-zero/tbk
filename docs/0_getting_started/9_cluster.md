# 构建集群

* 在集群的第一个Node上
    * 安装etcdadm & 编译
    * `etcdadm init`
    * `scp .tbk/etcdadm/pki/ca.* username@[IP_OF_NODE_X]:.tbk/etcdadm/pki/`
* 在其他集群上
    * 安装etcdadm & 编译
    * `etcdadm join https://[IP_OF_MASTER]:2380`
    * `etcdctl.sh member list``
    * `etcdctl.sh endpoint health --cluster``
