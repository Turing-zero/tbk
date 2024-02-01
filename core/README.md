# tbk core
* [x] semaphore
* [ ] plugins
* [ ] try unit test framework

---
## deps
---
```bash
# Ubuntu 22.04
apt install -y libfmt-dev libasio-dev nlohmann-json3-dev libyaml-cpp-dev protobuf-compiler
# needed by etcd-cpp-apiv3 if PROCESS_COMM is ON
apt install -y etcd
apt install -y libboost-all-dev libssl-dev libgrpc-dev libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc libcpprest-dev
# needed if BUILD_PYTHON_MODULE is ON
pip install "pybind11[global]"
```

## debug exports variable
* "TBK_DEBUG_MANAGER"
* "TBK_DEBUG_SOCKET"
* "TBK_DEBUG_CORE"
* "TBK_DEBUG_ETCD"