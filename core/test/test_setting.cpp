#include "tbk/setting.h"
#include "tbk/posix.h"
int main(){
    tbk::setting s("test");
    int a;
    s.loadParam(a,"a",1);
    std::cout << a << std::endl;
    std::array<int,3> b;
    s.loadParam(b,std::string("b"),{1,2,3});
    for(auto i:b){
        std::cout << i << std::endl;
    }
    std::array<int,3> c;
    s.loadParam(c,"c");
    for(auto i:c){
        std::cout << i << std::endl;
    }

    auto res = tbk::getClusterIP();
    std::cout << "res : " << res <<  std::endl;
    return 0;
}