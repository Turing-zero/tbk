#include <iostream>
#include <chrono>
#include <vector>
#include "tbk/tbk.h"

int main(){
    tbk::init("test_ping_multi_cpp");

    int count = 0;
    std::vector<tbk::Publisher> p_vec;

    for (int i=0;i<10;i++){
        p_vec.push_back(tbk::Publisher("ping_p"+std::to_string(i),"ping"+std::to_string(i)));
    }
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        count++;
        tbk::log("ping "+std::to_string(count));
        for(int i=0;i<10;i++){
            p_vec[i].publish("ping "+std::to_string(count)+" from "+std::to_string(i));
        }
    }
    return 0;
}