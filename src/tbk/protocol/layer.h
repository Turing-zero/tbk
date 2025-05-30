#pragma once
#include "../transport/interface.h"
#include "../discovery/interface.h"
#include <memory>

namespace tbk::protocol {

class ProtocolLayer {
public:
    ProtocolLayer(std::unique_ptr<transport::ITransport> transport,
                  std::unique_ptr<discovery::IDiscovery> discovery);
    
    // 统一的通信接口
    template<typename T>
    bool publish(const std::string& topic, const T& message);
    
    template<typename T>
    void subscribe(const std::string& topic, std::function<void(const T&)> callback);
    
    template<typename TReq, typename TResp>
    TResp request(const std::string& service, const TReq& request);
    
    template<typename TReq, typename TResp>
    void advertiseService(const std::string& service, 
                         std::function<TResp(const TReq&)> handler);
    
    // 参数服务
    template<typename T>
    void setParam(const std::string& key, const T& value);
    
    template<typename T>
    T getParam(const std::string& key, const T& defaultValue = T{});
    
private:
    std::unique_ptr<transport::ITransport> transport_;
    std::unique_ptr<discovery::IDiscovery> discovery_;
};

}