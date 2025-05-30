#pragma once
#include <string>
#include <vector>
#include <functional>

namespace tbk::discovery {

struct NodeInfo {
    std::string id;
    std::string address;
    std::vector<std::string> topics;
    std::map<std::string, std::string> metadata;
};

class IDiscovery {
public:
    virtual ~IDiscovery() = default;
    
    // 节点注册
    virtual bool registerNode(const NodeInfo& info) = 0;
    virtual bool unregisterNode(const std::string& nodeId) = 0;
    
    // 节点发现
    virtual std::vector<NodeInfo> discoverNodes(const std::string& topic = "") = 0;
    virtual NodeInfo getNodeInfo(const std::string& nodeId) = 0;
    
    // 事件回调
    using NodeEventCallback = std::function<void(const NodeInfo&, bool /*added*/)>;
    virtual void setNodeEventCallback(NodeEventCallback callback) = 0;
};

// ETCD实现
class EtcdDiscovery : public IDiscovery {
public:
    EtcdDiscovery(const std::string& etcdEndpoint);
    
    bool registerNode(const NodeInfo& info) override;
    bool unregisterNode(const std::string& nodeId) override;
    std::vector<NodeInfo> discoverNodes(const std::string& topic) override;
    NodeInfo getNodeInfo(const std::string& nodeId) override;
    void setNodeEventCallback(NodeEventCallback callback) override;
    
private:
    // ... existing ETCD implementation ...
};

}