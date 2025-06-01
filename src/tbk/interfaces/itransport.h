#pragma once
#include <memory>
#include <functional>
#include <vector>

namespace tbk::transport {

class ITransport {
public:
    virtual ~ITransport() = default;
    
    // 基础传输接口
    virtual bool send(const std::vector<uint8_t>& data, const std::string& target) = 0;
    virtual bool receive(std::vector<uint8_t>& data, std::string& source) = 0;
    
    // 异步接口
    using ReceiveCallback = std::function<void(const std::vector<uint8_t>&, const std::string&)>;
    virtual void setReceiveCallback(ReceiveCallback callback) = 0;
    
    // 连接管理
    virtual bool connect(const std::string& address) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};
}