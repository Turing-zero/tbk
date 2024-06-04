# 安装

## Ubuntu22.04安装依赖

TODO

## 编译
* build
```bash
cd '${PROJECT_ROOT}'
# get into CORE directory
cd core
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/tbk_core/install/path # -DBUILD_TEST=ON -DBUILD_PYTHON_MODULE=ON 
make -j12
# install the tbk_core library to the specified path
make install
```
* 通过增加`-DBUILD_TEST=ON`参数，编译测试程序
* 通过增加`-DBUILD_PYTHON_MODULE=ON`参数，编译python模块
* 通过增加`-DCMAKE_INSTALL_PREFIX=/tbk_core/install/path`参数，指定安装路径

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