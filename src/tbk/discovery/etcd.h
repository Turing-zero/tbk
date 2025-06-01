#pragma once
#include "tbk/interfaces/idiscovery.h"

namespace tbk::discovery {

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