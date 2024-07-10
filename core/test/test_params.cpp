#include "tbk/tbk.h"
#include "tbk/param.h"

enum class TestEnum{
    RED = 1,
    BLUE = 2,
    ORANGE = 3,
    CUSTOM = 100,
};
namespace tbk{
namespace param{
template<>
struct convert<TestEnum>{
    static TestEnum to(const std::string& s){
        if(s == "RED"){
            return TestEnum::RED;
        }else if(s == "BLUE"){
            return TestEnum::BLUE;
        }else if(s == "ORANGE"){
            return TestEnum::ORANGE;
        }else if(s == "CUSTOM"){
            return TestEnum::CUSTOM;
        }
        return TestEnum::RED;
    }
    static std::string from(const TestEnum& v){
        switch(v){
            case TestEnum::RED:
                return "RED";
            case TestEnum::BLUE:
                return "BLUE";
            case TestEnum::ORANGE:
                return "ORANGE";
            case TestEnum::CUSTOM:
                return "CUSTOM";
        }
        return "RED";
    }
    static std::string type(){
        return "Enum<TestEnum>";
    }
    static std::string info(){
        return "RED|BLUE|ORANGE|CUSTOM";
    }
};
}
}

template<typename T>
void _cb(const T& prev,const T& value){
    std::cout << "prev: " << tbk::param::convert<T>::from(prev) << ", new: " << tbk::param::convert<T>::from(value) << std::endl;
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

    tbk::Param<TestEnum> p4("enum_param",TestEnum::RED,_cb<TestEnum>);
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}