# 安装

> sudo apt install python3-gi gobject-introspection gir1.2-gtk-3.0 gir1.2-notify-0.7
> sudo apt install gir1.2-appindicator3-0.1
> sudo apt install gir1.2-ayatanaappindicator3-0.1  # in orangepi
## Ubuntu22.04安装依赖

* install deps
```bash
sudo apt install build-essential cmake libfmt-dev libprotobuf-dev libyaml-cpp-dev libboost-dev libboost-system-dev libboost-thread-dev libboost-random-dev pkg-config libgrpc++-dev pybind11-dev protobuf-compiler protobuf-compiler-grpc nlohmann-json3-dev curl openssh-server
```
* install etcd-cpp-apiv3
```bash
sudo apt install libcpprest-dev
# clone tag v0.15.4
git clone -b v0.15.4 https://github.com/etcd-cpp-apiv3/etcd-cpp-apiv3.git
cd etcd-cpp-apiv3
mkdir -p build
cd build
cmake ..
make -j`nproc`
make install # maybe need sudo
```
* install etcdadm
```bash
sudo snap install go --classic
git clone https://github.com/Turing-zero/etcdadm.git
cd etcdadm
# optional set proxy for module download
go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
# build etcdadm
make
# install the etcdadm binary to the specified path
make install
```

* install pybind11 for python module
```bash
sudo apt install python3-pip
pip install pybind11
```

## 安装tbkpy依赖
```bash
pip install git+https://github.com/Turing-zero/tbkpy.git
```

## 从源码编译tbk-core模块
```bash
## 下载源码（当前版本需要dev分支）
git clone --recurse-submodules https://github.com/Turing-zero/TBK.git
# git clone -b dev --recurse-submodules https://github.com/Turing-zero/TBK.git
export TBK_INSTALL_PATH={your/tbk_core/install/path} # your install path for tbk_core, such as $HOME/temp_usr_tbk or /usr/local/tbk
cd ${PROJECT_ROOT}/core
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$TBK_INSTALL_PATH # -DBUILD_TEST=ON
# full version
# cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/temp_usr_tbk -DBUILD_TEST=ON
cmake --build . -j`nproc`
# install the tbk_core library to the specified path
cmake --install .
```

* 通过增加`-DBUILD_TEST=ON`参数，编译测试程序
* 通过增加`-DCMAKE_INSTALL_PREFIX=$TBK_INSTALL_PATH`参数，指定安装路径

## **Optional**. 安装tbkpy-core（tbk-core的python接口）
```bash
export CMAKE_ARGS="-DCMAKE_PREFIX_PATH=$TBK_INSTALL_PATH"
pip install git+https://github.com/Turing-zero/tbkpy-core.git
```

## 启动ETCD后台

参考[配置集群](./9_cluster.md)章节

## 安装 && 测试
* 测试tbk-core
    ```bash
    # build demo for cpp testing
    cd ${PROJECT_ROOT}/demos/core_cpp_demo
    mkdir -p build
    cd build && cmake .. -DCMAKE_PREFIX_PATH=$TBK_INSTALL_PATH
    make
    ./TestTBK
    ```
* 测试tbkpy-core
    ```bash
    # 检查是否安装成功
    pip list | grep tbkpy
    # 应该有的输出
    # tbkpy                                0.x.x
    # tbkpy-core                           0.x.x
    # 开始ping/pong测试
    python3 -m tbkpy_core.test.ping
    # 另一个console下
    # python3 -m tbkpy_core.test.pong
    ```
