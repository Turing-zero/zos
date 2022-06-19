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
            std::string s(static_cast<const char*>(data.data()),data.size());
            zos::log("res : {}. data_size={}, s={}\n",res,data.size(),s);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    std::thread t2([&]{
        int count = 0;
        while(true){
            std::string s = fmt::format("format string:{}",count);
            zos::log("publish : {}\n",count++);
            p.publish(s.c_str(),s.size());
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            if(count > 6) break;
        }
    });
    t1.join();
    t2.join();
    return 0;
}