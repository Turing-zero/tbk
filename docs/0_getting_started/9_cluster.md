# 构建集群

<!-- * 在集群的第一个Node上
    * 安装etcdadm & 编译
    * `etcdadm init`
    * `scp .tbk/etcdadm/pki/ca.* username@[IP_OF_NODE_X]:.tbk/etcdadm/pki/`
* 在其他集群上
    * 安装etcdadm & 编译
    * `etcdadm join https://[IP_OF_MASTER]:2380`
    * `etcdctl.sh member list``
    * `etcdctl.sh endpoint health --cluster`` -->

在构建集群时，我们使用了etcdadm的工具，这在之前的[安装步骤](./0_installation.md)中已经提到了。tbk携带了使用python编写的用来调用etcdadm的GUI工具，你可以在终端下使用如下指令

```bash
python3 -m tbkpy.admin.main
```

你将会在任务栏中看到如下图标：

![tbkadm_0](../img/0_9_tbkadm_0.png)

这表示当前etcd为未启动状态（tbk后的数字为当前集群中的节点数量），单击图标并点击`Local Init New Cluster`，稍等片刻后可以看到图标变为绿色，这表示etcd已经启动，当前集群只有本机一个节点。

首次启动时，会直接下载etcd二进制文件并做缓存，之后的启动会直接使用缓存的二进制文件，这样可以加快集群构建的速度。

## 添加其他节点

TODO