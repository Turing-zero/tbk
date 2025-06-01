// #ifndef __TBK_RPC_H__
// #define __TBK_RPC_H__
// #include <thread>
// #include <grpcpp/ext/proto_server_reflection_plugin.h>
// #include <grpcpp/grpcpp.h>
// #include <grpcpp/health_check_service_interface.h>
// #include "tbk/type.h"
// #include "tbk/tbk_rpc.grpc.pb.h"
// namespace tbk{
// namespace rpc{
// // rpc server
// class zrpcImpl final : public zrpc::Service{
//     grpc::Status Add(grpc::ServerContext* context, const AddReq* req, AddRes* res) override;
//     grpc::Status Del(grpc::ServerContext* context, const DelReq* req, DelRes* res) override;
//     grpc::Status Get(grpc::ServerContext* context, const GetReq* req, GetRes* res) override;
// };
// class Server{
// public:
//     Server() = default;
//     ~Server();
//     void run();
//     void start();
//     void stop();
//     std::string _address;
//     int _port;
//     std::atomic<bool> _rpc_running = false;
// private:
//     std::unique_ptr<grpc::Server> _server;
//     std::thread _rpc_server_thread;
// };
// // rpc client
// class Client{
// public:
//     Client(std::shared_ptr<grpc::Channel> channel):stub_(zrpc::NewStub(channel)){}
//     bool add(const std::string& res_uuid, const SubscriberInfo& sub_info);
//     bool del(const std::string& res_uuid, const SubscriberInfo& sub_info);
//     bool get(const std::string& req_uuid,const std::string& res_uuid,const std::string& msg_uuid);
//     bool dealWithErrorCode(grpc::Status&);
// private:
//     std::unique_ptr<zrpc::Stub> stub_;
// };
// bool add(const std::string& s_address,const int s_port,const std::string& res_uuid, const SubscriberInfo& sub_info);
// bool del(const std::string& s_address,const int s_port,const std::string& res_uuid, const SubscriberInfo& sub_info);
// bool get(const std::string& s_address,const int s_port,const std::string& req_uuid,const std::string& res_uuid,const std::string& msg_uuid);
// } // namespace tbk::rpc
// } // namespace tbk
// #endif // __TBK_RPC_H__