// #include <thread>
// #include "tbk/manager.h"
// #include "tbk/rpc.h"
// namespace tbk{
// namespace rpc{
// // rpc server
// // using gServer = grpc::Server;
// using grpc::ServerBuilder;
// using grpc::ServerContext;
// using grpc::Status;
// Status zrpcImpl::Add(ServerContext* context, const AddReq* req, AddRes* res){
//     if(getenv("TBK_DEBUG_RPC")){
//         tbk::log("getreq Add : {}-{}-{}-{}-{}-{}\n",req->res_uuid(),req->sub().ep().address(),req->sub().ep().port(),req->sub().pid(),req->sub().msg_name(),req->sub().name());
//     }
//     auto _res = tbk::manager::_()->_rpc_api_add(req->res_uuid(),{req->sub().ep().address(),req->sub().ep().port(),req->sub().puuid(),req->sub().pid(),req->sub().uuid(),req->sub().name(),req->sub().msg_name(),nullptr,InfoFrom::OUTER});
//     res->set_req_uuid(req->sub().puuid());
//     res->set_res_uuid(req->res_uuid());
//     res->set_result(_res);
//     res->set_msg_name(req->sub().msg_name());
//     return Status::OK;
// }
// Status zrpcImpl::Del(ServerContext* context, const DelReq* req, DelRes* res){
//     if(getenv("TBK_DEBUG_RPC")){
//         tbk::log("getreq Del : {}-{}-{}-{}-{}-{}\n",req->res_uuid(),req->sub().ep().address(),req->sub().ep().port(),req->sub().pid(),req->sub().msg_name(),req->sub().name());
//     }
//     auto _res = tbk::manager::_()->_rpc_api_del(req->res_uuid(),{req->sub().ep().address(),req->sub().ep().port(),req->sub().puuid(),req->sub().pid(),req->sub().uuid(),req->sub().name(),req->sub().msg_name(),nullptr,InfoFrom::OUTER});
//     res->set_req_uuid(req->sub().puuid());
//     res->set_res_uuid(req->res_uuid());
//     res->set_result(_res);
//     res->set_msg_name(req->sub().msg_name());
//     return Status::OK;
// }
// Status zrpcImpl::Get(ServerContext* context, const GetReq* req, GetRes* res){
//     if(getenv("TBK_DEBUG_RPC")){
//         tbk::log("getreq Get : {}-{}-{}\n",req->req_uuid(),req->res_uuid(),req->linker_uuid());
//     }
//     return Status::OK;
// }

// void Server::run(){
//     if(std::getenv("TBK_DEBUG_RPC")){
//         tbk::status("_rpc_server start\n");
//     }
//     tbk::setting_g::_()->loadParam(_address,"manager.rpc_address","0.0.0.0");
//     std::random_device dev;
//     std::mt19937 rng(dev());
//     std::uniform_int_distribution<std::mt19937::result_type> dist(10000,65535);
//     do{
//         _port = dist(rng);
//     }while(tbk::checkPortUsage(_port));
//     auto server_address = fmt::format("{}:{}",_address,_port);
//     if(std::getenv("TBK_DEBUG_RPC")){
//         tbk::status("_rpc_server address : {}\n",server_address);
//     }
//     tbk::rpc::zrpcImpl service;
//     grpc::EnableDefaultHealthCheckService(true);
//     grpc::reflection::InitProtoReflectionServerBuilderPlugin();
//     grpc::ServerBuilder builder;
//     builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
//     builder.RegisterService(&service);
//     // Finally assemble the server.
//     _server = (builder.BuildAndStart());
//     _rpc_running=true;
//     _server->Wait();
//     _rpc_running=false;
//     _server.reset();
//     tbk::status("_rpc_server exit\n");
// }
// void Server::start(){
//     if(!_rpc_running){
//         _rpc_server_thread = std::thread(&Server::run,this);
//     }
// }
// void Server::stop(){
//     if(_rpc_running){
//         _server->Shutdown();
//     }
// }
// Server::~Server(){
//     stop();
//     if(_rpc_server_thread.joinable()){
//         _rpc_server_thread.join();
//     }
// }
// // rpc client
// using grpc::Channel;
// using grpc::ClientContext;
// using grpc::Status;
// using grpc::StatusCode;
// bool Client::dealWithErrorCode(Status& status){
//     switch(status.error_code()){
//         case StatusCode::OK:
//             return true;
//         case StatusCode::UNAVAILABLE:
//             // server unavailable
//             return false;
//         default:
//             tbk::error("rpc error : {}:{}\n",status.error_code(),status.error_message());
//             return false;
//     }
//     return false;
// }
// bool Client::add(const std::string& res_uuid, const SubscriberInfo& sub_info){
//     if(getenv("TBK_DEBUG_RPC")){
//         tbk::log("rpc add req : {}-{}-{}-{}-{}-{}\n",res_uuid,sub_info.ip,sub_info.port,sub_info.pid,sub_info.msg_name,sub_info.name);
//     }
//     AddReq req;
//     req.set_res_uuid(res_uuid);
//     auto sub = req.mutable_sub();
//     auto _endpoint = sub->mutable_ep();
//     _endpoint->set_address(sub_info.ip);
//     _endpoint->set_port(sub_info.port);
//     sub->set_puuid(sub_info.puuid);
//     sub->set_pid(sub_info.pid);
//     sub->set_uuid(sub_info.uuid);
//     sub->set_msg_name(sub_info.msg_name);
//     sub->set_name(sub_info.name);

//     AddRes res;
//     ClientContext context;
//     auto status = stub_->Add(&context,req,&res);

//     if(!status.ok()){
//         return dealWithErrorCode(status);
//     }
//     if(getenv("TBK_DEBUG_RPC")){
//         tbk::log("getres : {}-{}-{}-{}\n",res.req_uuid(),res.res_uuid(),res.result(),res.msg_name());
//     }
//     return res.result();
// }
// bool Client::del(const std::string& res_uuid, const SubscriberInfo& sub_info){
//     if(getenv("TBK_DEBUG_RPC")){
//         tbk::log("rpc del req : {}-{}-{}-{}-{}-{}\n",res_uuid,sub_info.ip,sub_info.port,sub_info.pid,sub_info.msg_name,sub_info.name);
//     }
//     DelReq req;
//     req.set_res_uuid(res_uuid);
//     auto sub = req.mutable_sub();
//     auto _endpoint = sub->mutable_ep();
//     _endpoint->set_address(sub_info.ip);
//     _endpoint->set_port(sub_info.port);
//     sub->set_puuid(sub_info.puuid);
//     sub->set_pid(sub_info.pid);
//     sub->set_msg_name(sub_info.msg_name);
//     sub->set_uuid(sub_info.uuid);
//     sub->set_name(sub_info.name);
//     DelRes res;
//     ClientContext context;
//     auto status = stub_->Del(&context,req,&res);
//     if(!status.ok()){
//         return dealWithErrorCode(status);
//     }
//     if(getenv("TBK_DEBUG_RPC")){
//         tbk::log("getres : {}-{}-{}-{}\n",res.req_uuid(),res.res_uuid(),res.result(),res.msg_name());
//     }
//     return res.result();
// }
// bool Client::get(const std::string& req_uuid,const std::string& res_uuid,const std::string& linker_uuid){
//     return false;
// }
// bool add(const std::string& s_address,const int s_port,const std::string& res_uuid, const SubscriberInfo& sub_info){
//     Client client(grpc::CreateChannel(s_address+":"+std::to_string(s_port),grpc::InsecureChannelCredentials()));
//     return client.add(res_uuid,sub_info);
// }
// bool del(const std::string& s_address,const int s_port,const std::string& res_uuid, const SubscriberInfo& sub_info){
//     Client client(grpc::CreateChannel(s_address+":"+std::to_string(s_port),grpc::InsecureChannelCredentials()));
//     return client.del(res_uuid,sub_info);
// }
// bool get(const std::string& s_address,const int s_port,const std::string& req_uuid,const std::string& res_uuid,const std::string& linker_uuid){
//     Client client(grpc::CreateChannel(s_address+":"+std::to_string(s_port),grpc::InsecureChannelCredentials()));
//     return client.get(req_uuid,res_uuid,linker_uuid);
// }
// } // namespace tbk::rpc
// } // namespace tbk