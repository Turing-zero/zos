#include <iostream>
#include "zos/core.h"

void _cb1(const zos::Data& data){
    zos::log("in cb1!!! size:{}\n",data.size());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
void _cb2(const zos::Data& data){
    zos::log("in cb2!!! size:{}\n",data.size());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
int main(){
    zos::Subscriber s1("test",_cb1);
    zos::Subscriber s2("test",_cb2);
    zos::Publisher p("test");
    p.link(&s1,&s2);
    std::thread t([&]{
        int count = 0;
        while(true){
            p.publish(nullptr,0);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if(++count > 2) break;
        }
    });
    t.join();
    return 0;
}