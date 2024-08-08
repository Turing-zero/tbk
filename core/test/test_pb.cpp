#include "tbk/tbk_test.pb.h"
#include <fmt/format.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void print(const tbk::pb::Pack& pack){
    fmt::print("type:{},name:{},d_size:{}\n",pack.type(),pack.name(),pack.data().length());

    if(pack.type() == tbk::pb::Pack::JSON){
        auto data = pack.data();
        fmt::print("data : {}\n",json::parse(data).dump());
    }else if(pack.type() == tbk::pb::Pack::JSON_CBOR){
        auto data = json::from_cbor(pack.data());
        fmt::print("data : {}\n",data.dump());
    }
}

void print_msg(const tbk::pb::Msg4Test& msg){
    fmt::print("name:{},nums:",msg.name());
    for(int i=0;i<msg.nums_size();i++){
        fmt::print("{} ",msg.nums(i));
    }
    fmt::print("\n");
}

int main(){
    tbk::pb::Pack pack;
    pack.set_type(tbk::pb::Pack::JSON);
    pack.set_name("pack1");

    json j_object = {{"a",123},{"b","this's a string."}};
    
    pack.set_data(j_object.dump());


    tbk::pb::Pack pack2;
    pack2.set_type(tbk::pb::Pack::PB);
    pack2.set_name("pack2:Msg4Test");
    tbk::pb::Msg4Test msg;
    msg.set_name("i'm a testing msg.");
    for(int i=0;i<5;i++){
        msg.add_nums(i+1.0);
    }
    msg.SerializeToString(pack2.mutable_data());


    tbk::pb::Pack pack3;
    pack3.set_type(tbk::pb::Pack::JSON_CBOR);
    pack3.set_name("pack3");
    j_object["a"] = 234;
    auto cbor_data = json::to_cbor(j_object);
    pack3.set_data(std::string(cbor_data.begin(),cbor_data.end()));


    print(pack);

    print(pack2);

    tbk::pb::Msg4Test msg4test;
    msg4test.ParseFromString(pack2.data());
    print_msg(msg4test);

    print(pack3);
}