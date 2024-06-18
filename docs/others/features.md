# 功能清单 / TodoList
* tbk manager
    * [ ] 时间戳相关（时间同步，时间戳校准）
* tbk comm
    * [ ] 支持其他通信方式（SharedMem，UDP，TCP，针对LTE的通信方案？）
    * [ ] 增加通信设置（保完整性tcp/保实时性udp）
    * [ ] 增加protocol设置（是否严格匹配protocol格式-json/yaml/raw/pb-msg）
    * [ ] 自带丢包率检测（根据通信配置进行方式切换）
* tbk thread
    * [x] publisher (alter for string map query)
    * [x] threadpool & msg callback
* tbk process
    * [x] discover in localhost
    * [x] UDP port
    * [x] protocol&msg name
    * [ ] add plugin to normal udp message
* tbk LAN
    * [ ] connect to other PC
    * [ ] choose msg (prefer for each msg)
* tbk gui
    * [ ] viz for msg info in different level (thread/process/LAN)
    * [ ] config each msg
    * [ ] viz for msg
* tbk param 参数服务器
    * [ ] param in different scope/level
    * [x] param in process
    * [x] param in ETCD
    * [x] setparam callback
    * [x] setparam & getparam cmd
    * [ ] gui support
* tbk cmd
    * [ ] tab auto complete
    * [ ] auto install & auto set PATH
* tbk plugin
    * [ ] ros
    * [ ] Apollo Cyber RT
    * [ ] LCM
* others
    * [x] 优化编译方式
    * [x] add doc
    * [ ] pip install
    * [ ] 安全性
* optimization
    * [ ] 目前单机的PingPong不具备优势
