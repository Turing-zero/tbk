#ifndef __TBK_MESSAGING_PUBLISHER_H__
#define __TBK_MESSAGING_PUBLISHER_H__

#include <functional>
#include <memory>
#include <vector>
#include "tbk/messaging/message_types.h"
#include "tbk/transport/transport_interface.h"
#include "tbk/discovery/discovery_interface.h"
#include "tbk/common/result.h"
#include "tbk/common/endpoint.h"

namespace tbk {
namespace messaging {

// 发布者配置
struct PublisherConfig {
    std::string topic;
    std::string namespace_ = "default";
    CommLevel commLevel = CommLevel::Localhost;
    bool enableReliability = false;
    uint32_t maxRetries = 3;
    std::chrono::milliseconds retryInterval{100};
};

// 类型安全的发布者接口
template<typename MessageType>
class TypedPublisher {
    static_assert(std::is_base_of_v<IMessage, MessageType>, "MessageType must inherit from IMessage");
    
public:
    TypedPublisher(const PublisherConfig& config,
                   std::shared_ptr<transport::ITransport> transport,
                   std::shared_ptr<discovery::IDiscovery> discovery)
        : config_(config), transport_(transport), discovery_(discovery) {}
    
    virtual ~TypedPublisher() = default;
    
    // 发布消息
    virtual Result<void> publish(const MessageType& message) {
        auto serialized = message.serialize();
        auto subscribers = discovery_->findSubscribers(config_.topic, config_.namespace_);
        
        if (subscribers.isError()) {
            return makeError(ErrorCode::InternalError, "Failed to find subscribers", subscribers.error().toString());
        }
        
        return transport_->broadcast(serialized, subscribers.value());
    }
    
    // 异步发布
    virtual void publishAsync(const MessageType& message, 
                             std::function<void(Result<void>)> callback) {
        // 在线程池中执行
        std::thread([this, message, callback]() {
            auto result = publish(message);
            callback(result);
        }).detach();
    }
    
    // 获取配置
    const PublisherConfig& getConfig() const { return config_; }
    
    // 更新配置
    Result<void> updateConfig(const PublisherConfig& newConfig) {
        config_ = newConfig;
        return discovery_->updatePublisher(config_);
    }
    
private:
    PublisherConfig config_;
    std::shared_ptr<transport::ITransport> transport_;
    std::shared_ptr<discovery::IDiscovery> discovery_;
};

// 订阅者配置
struct SubscriberConfig {
    std::string topic;
    std::string namespace_ = "default";
    CommLevel commLevel = CommLevel::Localhost;
    uint32_t bufferSize = 100;
    bool enableOrdering = false;
    std::chrono::milliseconds timeout{5000};
};

// 类型安全的订阅者接口
template<typename MessageType>
class TypedSubscriber {
    static_assert(std::is_base_of_v<IMessage, MessageType>, "MessageType must inherit from IMessage");
    
public:
    using MessageHandler = std::function<void(const MessageType&)>;
    using ErrorHandler = std::function<void(const Error&)>;
    
    TypedSubscriber(const SubscriberConfig& config,
                    std::shared_ptr<transport::ITransport> transport,
                    std::shared_ptr<discovery::IDiscovery> discovery)
        : config_(config), transport_(transport), discovery_(discovery) {
        
        // 设置传输层消息处理器
        transport_->setMessageHandler([this](const std::vector<uint8_t>& data, const Endpoint& sender) {
            handleIncomingMessage(data, sender);
        });
    }
    
    virtual ~TypedSubscriber() = default;
    
    // 设置消息处理器
    void setMessageHandler(MessageHandler handler) {
        messageHandler_ = handler;
    }
    
    // 设置错误处理器
    void setErrorHandler(ErrorHandler handler) {
        errorHandler_ = handler;
    }
    
    // 启动订阅
    Result<void> start() {
        auto result = transport_->start();
        if (result.isError()) {
            return result;
        }
        
        return discovery_->registerSubscriber(config_);
    }
    
    // 停止订阅
    void stop() {
        transport_->stop();
        discovery_->unregisterSubscriber(config_);
    }
    
    // 同步接收消息（阻塞）
    Result<MessageType> receive(std::chrono::milliseconds timeout = std::chrono::milliseconds::max()) {
        std::unique_lock<std::mutex> lock(messageMutex_);
        
        if (timeout == std::chrono::milliseconds::max()) {
            messageCondition_.wait(lock, [this] { return !messageQueue_.empty(); });
        } else {
            if (!messageCondition_.wait_for(lock, timeout, [this] { return !messageQueue_.empty(); })) {
                return makeError<MessageType>(ErrorCode::TimeoutError, "Receive timeout");
            }
        }
        
        auto message = std::move(messageQueue_.front());
        messageQueue_.pop();
        return makeSuccess(std::move(message));
    }
    
private:
    void handleIncomingMessage(const std::vector<uint8_t>& data, const Endpoint& sender) {
        MessageType message;
        auto result = message.deserialize(data);
        
        if (result.isError()) {
            if (errorHandler_) {
                errorHandler_(result.error());
            }
            return;
        }
        
        // 如果设置了消息处理器，直接调用
        if (messageHandler_) {
            messageHandler_(message);
        } else {
            // 否则放入队列供同步接收
            std::lock_guard<std::mutex> lock(messageMutex_);
            messageQueue_.push(std::move(message));
            messageCondition_.notify_one();
        }
    }
    
    SubscriberConfig config_;
    std::shared_ptr<transport::ITransport> transport_;
    std::shared_ptr<discovery::IDiscovery> discovery_;
    
    MessageHandler messageHandler_;
    ErrorHandler errorHandler_;
    
    std::mutex messageMutex_;
    std::condition_variable messageCondition_;
    std::queue<MessageType> messageQueue_;
};

} // namespace messaging
} // namespace tbk

#endif // __TBK_MESSAGING_PUBLISHER_H__