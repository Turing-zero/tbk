#include "tbk/posix.h"
#include <unistd.h>
#include <string>
#include <vector>
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <fmt/format.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <pwd.h>
namespace tbk{
// write a function that execute a shell command and return the result
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
std::string exec(const std::string& cmd){
    return exec((cmd).c_str());
}
// get the current process id
int getProcessId(){
    return getpid();
}
// get the current user home directory
std::string getHomeDir(){
    std::string homedir = getenv("HOME");
    if(homedir.empty()){
        homedir = getpwuid(getuid())->pw_dir;
    }
    return homedir;
}
// get the current process name
std::string getProcessName(){
    auto res = exec(fmt::format("ps -p {} -o comm=",getProcessId()));
    res.pop_back();
    return res;
}
// write a function that return all available network interfaces ip
std::vector<std::string> getNetworkInterfacesIP(const int ipType){
    struct ifaddrs* ptr_ifaddrs = nullptr;
    auto result = getifaddrs(&ptr_ifaddrs);
    if(result != 0){
        throw std::runtime_error("getifaddrs() failed!");
    }
    std::vector<std::string> res;
    for(auto ptr = ptr_ifaddrs;ptr != nullptr;ptr = ptr->ifa_next){
        if(ptr->ifa_addr == nullptr){
            continue;
        }
        if(ipType&IPV4 && ptr->ifa_addr->sa_family == AF_INET){
            char buffer[INET_ADDRSTRLEN] = {0, };
            auto ptr_in_addr = reinterpret_cast<struct sockaddr_in*>(ptr->ifa_addr);
            auto ptr_in_addr_str = inet_ntop(AF_INET,&ptr_in_addr->sin_addr,buffer,INET_ADDRSTRLEN);
            if(ptr_in_addr_str != nullptr){
                res.push_back(ptr_in_addr_str);
            }
        }else if(ipType&IPV6 && ptr->ifa_addr->sa_family == AF_INET6){
            char buffer[INET6_ADDRSTRLEN] = {0, };
            auto ptr_in_addr = reinterpret_cast<struct sockaddr_in6*>(ptr->ifa_addr);
            auto ptr_in_addr_str = inet_ntop(AF_INET6,&ptr_in_addr->sin6_addr,buffer,INET6_ADDRSTRLEN);
            if(ptr_in_addr_str != nullptr){
                res.push_back(ptr_in_addr_str);
            }
        }
    }
    freeifaddrs(ptr_ifaddrs);
    return res;
}
std::string getClusterIP(){
    auto ifs = getNetworkInterfacesIP(IPV4);
    auto cluster_nodes_urls = tbk::exec("python3 -m tbkpy.tools.tool_etcd info --format {ENDPOINT}");
    std::regex expr("http(s)://(.*):");
    std::smatch match;
    while(std::regex_search(cluster_nodes_urls,match,expr)){
        auto url = match[2].str();
        if(std::find(ifs.begin(),ifs.end(),url) != ifs.end()){
            return url;
        }
        cluster_nodes_urls = match.suffix().str();
    }
    return "";
}
// write a function that check if a given address and port is in use
bool checkPortUsage(const std::string& address, const int port){
    auto res = exec(fmt::format("ss -anp 2>&1 | grep -w {} | grep -w {}",address,port));
    return res.empty() ? false : true;
}
// write a function that check if a given port is in use
bool checkPortUsage(const int port){
    auto res = exec(fmt::format("ss -anp 2>&1 | grep -w {}",port));
    return res.empty() ? false : true;
}
std::string generateUUID(){
    auto res = exec("uuidgen");
    res.pop_back();
    return res;
}
std::string _match(const std::string& str, const std::string& expr){
    std::regex _expr(expr);
    std::smatch _m;
    if(std::regex_search(str,_m,_expr)){
        return _m[1].str();
    }
    return "";
}
DeviceInfo getDeviceInfo(){
    std::string all_info = exec("hostnamectl");
    DeviceInfo info;
    info.uuid = ""; // not implemented yet
    // example : 
    //    Static hostname: orangepicm4
    //          Icon name: computer
    //         Machine ID: 72d280cae467476b8430f3fb04b8d867
    //            Boot ID: 21b056874b7447328d4e9c327d273b72
    //   Operating System: Orange Pi 1.0.4 Bullseye
    //             Kernel: Linux 5.10.160-rockchip-rk356x
    //       Architecture: arm64
    info.static_hostname = _match(all_info,"Static hostname: (.*)");
    info.icon_name = _match(all_info,"Icon name: (.*)");
    info.machine_id = _match(all_info,"Machine ID: (.*)");
    info.boot_id = _match(all_info,"Boot ID: (.*)");
    info.os = _match(all_info,"Operating System: (.*)");
    info.kernel = _match(all_info,"Kernel: (.*)");
    info.arch = _match(all_info,"Architecture: (.*)");
    return info;
}
} // namespace tbk