#ifndef __TBK_MESSAGE_TYPES_H__
#define __TBK_MESSAGE_TYPES_H__

#include <string>
#include <vector>
#include <memory>
#include <typeinfo>
#include <typeindex>
#include <google/protobuf/message.h>
#include <google/protobuf/any.pb.h>
#include "tbk/common/result.h"

namespace tbk {
namespace messaging {

// 消息类型接口
class IMessage {
public:
    virtual ~IMessage() = default;
    virtual std::string getTypeName() const = 0;
    virtual std::vector<uint8_t> serialize() const = 0;
    virtual Result<void> deserialize(const std::vector<uint8_t>& data) = 0;
    virtual std::unique_ptr<IMessage> clone() const = 0;
};

// Protocol Buffers消息包装
template<typename T>
class ProtobufMessage : public IMessage {
    static_assert(std::is_base_of_v<google::protobuf::Message, T>, "T must be a protobuf message");
    
public:
    ProtobufMessage() = default;
    ProtobufMessage(const T& msg) : message_(msg) {}
    ProtobufMessage(T&& msg) : message_(std::move(msg)) {}
    
    std::string getTypeName() const override {
        return T::descriptor()->full_name();
    }
    
    std::vector<uint8_t> serialize() const override {
        std::string serialized = message_.SerializeAsString();
        return std::vector<uint8_t>(serialized.begin(), serialized.end());
    }
    
    Result<void> deserialize(const std::vector<uint8_t>& data) override {
        std::string str(data.begin(), data.end());
        if (!message_.ParseFromString(str)) {
            return makeError(ErrorCode::InvalidArgument, "Failed to parse protobuf message");
        }
        return makeSuccess();
    }
    
    std::unique_ptr<IMessage> clone() const override {
        return std::make_unique<ProtobufMessage<T>>(message_);
    }
    
    const T& getMessage() const { return message_; }
    T& getMessage() { return message_; }
    
private:
    T message_;
};

// 原始字节消息
class RawMessage : public IMessage {
public:
    RawMessage() = default;
    RawMessage(const std::vector<uint8_t>& data) : data_(data) {}
    RawMessage(std::vector<uint8_t>&& data) : data_(std::move(data)) {}
    
    std::string getTypeName() const override {
        return "tbk.messaging.RawMessage";
    }
    
    std::vector<uint8_t> serialize() const override {
        return data_;
    }
    
    Result<void> deserialize(const std::vector<uint8_t>& data) override {
        data_ = data;
        return makeSuccess();
    }
    
    std::unique_ptr<IMessage> clone() const override {
        return std::make_unique<RawMessage>(data_);
    }
    
    const std::vector<uint8_t>& getData() const { return data_; }
    std::vector<uint8_t>& getData() { return data_; }
    
private:
    std::vector<uint8_t> data_;
};

// 消息工厂
class MessageFactory {
public:
    template<typename T>
    static void registerType() {
        static_assert(std::is_base_of_v<IMessage, T>, "T must inherit from IMessage");
        auto creator = []() -> std::unique_ptr<IMessage> {
            return std::make_unique<T>();
        };
        instance().creators_[std::type_index(typeid(T))] = creator;
    }
    
    template<typename T>
    static std::unique_ptr<T> create() {
        auto it = instance().creators_.find(std::type_index(typeid(T)));
        if (it != instance().creators_.end()) {
            return std::unique_ptr<T>(static_cast<T*>(it->second().release()));
        }
        return nullptr;
    }
    
    static std::unique_ptr<IMessage> createByTypeName(const std::string& typeName);
    
private:
    static MessageFactory& instance() {
        static MessageFactory factory;
        return factory;
    }
    
    std::map<std::type_index, std::function<std::unique_ptr<IMessage>()>> creators_;
};

} // namespace messaging
} // namespace tbk

#endif // __TBK_MESSAGE_TYPES_H__