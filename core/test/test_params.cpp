#include "tbk/tbk.h"
#include "tbk/param.h"

template<typename T>
void _cb(const T& prev,const T& value){
    std::cout << "prev: " << prev << ", new: " << value << std::endl;
}
int main(int argc, char *argv[]){
    std::string symbol = "default";
    if (argc>1) symbol = argv[1]; 
    tbk::init("TestParamNode");
    // get param with default value, if not exist, create it
    tbk::param::Int p("int_param",1);
    std::cout << p.name() << " " << p.get() << std::endl;
    // get param strightly, if not exist, warning
    tbk::param::String p2("string_param",_cb<std::string>);
    p2.set(symbol);
    std::cout << p2.name() << " " << p2.get() << std::endl;
    // get param with callback, called when param changed
    tbk::param::Double p3("double_param",1.2,_cb<double>);
    p3.set(3.4);
    std::cout << p3.name() << " " << p3.get() << std::endl;
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}