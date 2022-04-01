#include <iostream>
#include "zos/core.h"

int main(){
    zos::log("hello zos!\n");
    zos::Subscriber<3> s("test");
    zos::Publisher p("test");
    p.link(&s);
    p.publish(nullptr,0);
    std::thread t1([&]{
        zos::Data data;
        while(true){
            auto res = s.try_get(data);
            zos::log("res : {}. data_size={}\n",res,data.size());
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    std::thread t2([&]{
        int count = 0;
        while(true){
            p.publish(nullptr,0);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            if(++count > 10) break;
        }
    });
    t1.join();
    t2.join();
    return 0;
}