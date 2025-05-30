#include "tbk/tbk.h"
namespace tbk{
void init(const std::string& _name,const std::string& _namespace){
    if(getenv("http_proxy")){
        tbk::error("It looks like you have proxy enabled, to use with multi process comm, please close with `unset http_proxy`\n");
    }
    auto name = fmt::format("{}_{}",_name,_namespace);
    tbk::manager::_(_name.c_str(),_namespace.c_str());
    // tbk::setting_g::_(name.c_str());
}
} // namespace tbk