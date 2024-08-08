# 安装

## Ubuntu22.04安装依赖

* install deps
```bash
sudo apt install build-essential cmake libfmt-dev libprotobuf-dev libyaml-cpp-dev libboost-dev libboost-system-dev libboost-thread-dev libboost-random-dev pkg-config libgrpc++-dev pybind11-dev protobuf-compiler protobuf-compiler-grpc nlohmann-json3-dev curl openssh-server
```
* install etcd-cpp-apiv3
```bash
sudo apt install libcpprest-dev
git clone https://github.com/etcd-cpp-apiv3/etcd-cpp-apiv3.git
cd etcd-cpp-apiv3
# switch to tag v0.15.4
git checkout v0.15.4
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
make
# install the etcdadm binary to the specified path
make install
```

* install pybind11 for python module
```bash
sudo apt install python3-pip
pip install pybind11
```

## Clone代码
```bash
git clone --recurse-submodules https://github.com/Turing-zero/TBK.git
# git clone -b dev --recurse-submodules https://github.com/Turing-zero/TBK.git
```

## 编译
```bash
cd ${PROJECT_ROOT}
# get into CORE directory
cd core
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/tbk_core/install/path # -DBUILD_TEST=ON -DBUILD_PY=ON 
# full version
# cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/temp_usr_tbk -DBUILD_TEST=ON -DBUILD_PY=ON
make -j12
# install the tbk_core library to the specified path
make install
```

* 通过增加`-DBUILD_TEST=ON`参数，编译测试程序
* 通过增加`-DBUILD_PY=ON`参数，编译python模块
* 通过增加`-DCMAKE_INSTALL_PREFIX=/tbk_core/install/path`参数，指定安装路径

## 安装 && 测试
```bash
# build demo for cpp testing
cd ${PROJECT_ROOT}/demos/core_cpp_demo
mkdir -p build
cd build && cmake .. -DCMAKE_PREFIX_PATH=/tbk_core/install/path
make
./TestTBK

# install python module
cd ${PROJECT_ROOT}/core/build/pymodule
python3 -m pip install -e .
# check if the installation is successful
python3 -c "import tbkpy"
```