#ifndef __TBK_TRANSPORT_INTERFACE_H__
#define __TBK_TRANSPORT_INTERFACE_H__

#include <functional>
#include <memory>
#include <vector>
#include "tbk/common/result.h"
#include "tbk/common/endpoint.h"

namespace tbk {
namespace transport {

// 传输层抽象接口
class ITransport {
public:
    using MessageHandler = std::function<void(const std::vector<uint8_t>&, const Endpoint&)>;
    
    virtual ~ITransport() = default;
    
    // 发送消息
    virtual Result<void> send(const std::vector<uint8_t>& data, const Endpoint& target) = 0;
    
    // 广播消息
    virtual Result<void> broadcast(const std::vector<uint8_t>& data, const std::vector<Endpoint>& targets) = 0;
    
    // 设置消息接收处理器
    virtual void setMessageHandler(MessageHandler handler) = 0;
    
    // 启动传输层
    virtual Result<void> start() = 0;
    
    // 停止传输层
    virtual void stop() = 0;
    
    // 获取本地端点信息
    virtual Endpoint getLocalEndpoint() const = 0;
};

// UDP传输实现
class UdpTransport : public ITransport {
public:
    UdpTransport(const std::string& bind_ip = "0.0.0.0", uint16_t bind_port = 0);
    ~UdpTransport() override;
    
    Result<void> send(const std::vector<uint8_t>& data, const Endpoint& target) override;
    Result<void> broadcast(const std::vector<uint8_t>& data, const std::vector<Endpoint>& targets) override;
    void setMessageHandler(MessageHandler handler) override;
    Result<void> start() override;
    void stop() override;
    Endpoint getLocalEndpoint() const override;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

// 进程内传输实现
class InProcessTransport : public ITransport {
public:
    InProcessTransport();
    ~InProcessTransport() override;
    
    Result<void> send(const std::vector<uint8_t>& data, const Endpoint& target) override;
    Result<void> broadcast(const std::vector<uint8_t>& data, const std::vector<Endpoint>& targets) override;
    void setMessageHandler(MessageHandler handler) override;
    Result<void> start() override;
    void stop() override;
    Endpoint getLocalEndpoint() const override;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace transport
} // namespace tbk

#endif // __TBK_TRANSPORT_INTERFACE_H__