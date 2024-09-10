#ifndef __TBK_POSIX_H__
#define __TBK_POSIX_H__
#include <string>
#include <vector>
namespace tbk{
struct DeviceInfo{
    std::string uuid;
    // got all info from 'hostnamectl' command
    std::string static_hostname;
    std::string icon_name;
    std::string machine_id;
    std::string boot_id;
    std::string os;
    std::string kernel;
    std::string arch;
};
const int IPV4 = 0x01<<0;
const int IPV6 = 0x01<<1;
std::string exec(const char*);
std::string exec(const std::string&);
int getProcessId();
std::string getHomeDir();
std::string getProcessName();
std::string generateUUID();
std::vector<std::string> getNetworkInterfacesIP(const int ipType = IPV4);
std::string getClusterIP();
bool checkPortUsage(const std::string& address, const int port);
bool checkPortUsage(const int port);
DeviceInfo getDeviceInfo();
} // namespace tbk
#endif // __TBK_POSIX_H__
