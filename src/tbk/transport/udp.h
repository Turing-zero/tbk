#pragma once
#include "tbk/interfaces/itransport.h"

namespace tbk::transport {

// UDP实现
class UdpTransport : public ITransport {
public:
    bool send(const std::vector<uint8_t>& data, const std::string& target) override;
    bool receive(std::vector<uint8_t>& data, std::string& source) override;
    void setReceiveCallback(ReceiveCallback callback) override;
    bool connect(const std::string& address) override;
    void disconnect() override;
    bool isConnected() const override;
    
private:
    // ... existing UDP implementation ...
};

}