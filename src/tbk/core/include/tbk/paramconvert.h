#ifndef __TBK_PARAMCONVERT_H__
#define __TBK_PARAMCONVERT_H__
#include <string>
#include <stdexcept>
#include "tbk/log.h"
namespace tbk{
namespace param{
template<typename T>
struct convert{};
template<>
struct convert<int>{
    static int to(const std::string& s){
        try{
            return std::stoi(s);
        }catch (std::invalid_argument const& ex){
            tbk::error("std::stoi - invalid_argument::what(): {}, v: {}\n", ex.what(), s);
        }catch (std::out_of_range const& ex){
            tbk::error("std::stoi - out_of_range::what(): {}, v:{}\n", ex.what(), s);
        }
        return 0;
    }
    static std::string from(const int& v){
        return std::to_string(v);
    }
    static std::string type(){
        return "int";
    }
    static std::string info(){
        return "int";
    }
};
template<>
struct convert<double>{
    static double to(const std::string& s){
        try{
            return std::stod(s);
        }catch (std::invalid_argument const& ex){
            tbk::error("std::stod - invalid_argument::what(): {}, v: {}\n", ex.what(), s);
        }catch (std::out_of_range const& ex){
            tbk::error("std::stod - out_of_range::what(): {}, v:{}\n", ex.what(), s);
        }
        return 0.0;
    }
    static std::string from(const double& v){
        return std::to_string(v);
    }
    static std::string type(){
        return "double";
    }
    static std::string info(){
        return "double";
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
    static std::string type(){
        return "string";
    }
    static std::string info(){
        return "string";
    }
};
template<>
struct convert<bool>{
    static bool to(const std::string& s){
        std::string lower(s.length(),0);
        std::transform(s.begin(), s.end(), lower.begin(), [](unsigned char c){ return std::tolower(c);});
        return lower=="true";
    }
    static std::string from(const bool& v){
        return v?"True":"False";
    }
    static std::string type(){
        return "bool";
    }
    static std::string info(){
        return "bool";
    }
};
} // namespace tbk::param
} // namespace tbk
#endif // __TBK_PARAMCONVERT_H__