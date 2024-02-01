#ifndef __TBK_PARAMCONVERT_H__
#define __TBK_PARAMCONVERT_H__
#include <string>
namespace tbk{
namespace param{
template<typename T>
struct convert{};
template<>
struct convert<int>{
    static int to(const std::string& s){
        return std::stoi(s);
    }
    static std::string from(const int& v){
        return std::to_string(v);
    }
};
template<>
struct convert<double>{
    static double to(const std::string& s){
        return std::stod(s);
    }
    static std::string from(const double& v){
        return std::to_string(v);
    }
};
template<>
struct convert<std::string>{
    static std::string to(const std::string& s){
        return s;
    }
    static std::string from(const std::string& v){
        return v;
    }
};
template<>
struct convert<bool>{
    static bool to(const std::string& s){
        return s=="true";
    }
    static std::string from(const bool& v){
        return v?"true":"false";
    }
};
} // namespace tbk::param
} // namespace tbk
#endif // __TBK_PARAMCONVERT_H__