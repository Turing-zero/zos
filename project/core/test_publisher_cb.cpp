#include <iostream>
#include "zos/core.h"
#include <ctime>
#include <iostream>
#include <unistd.h>

std::string gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

void _cb1(const zos::Data& data){
    zos::log("in cb1!!! size:{}\n",data.size());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
void _cb2(const zos::Data& data){
    zos::log("in cb2!!! size:{}\n",data.size());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
int main(){
    srand((unsigned)time(NULL) * getpid());
    zos::Subscriber<1> s1("test",_cb1);
    zos::Subscriber<1> s2("test",_cb2);
    auto* p = new zos::Publisher("test");
    p->link(&s1,&s2);
    std::thread t[10];
    for(int i=0;i<10;i++){
        t[i] = std::thread([&]{
            int count = 0;
            while(true){
                auto str = gen_random(count);
                p->publish(str.c_str(),str.size());
                zos::log("in publish!!! size:{}\n",str.size());
    //            std::this_thread::sleep_for(std::chrono::milliseconds(200));
                if(++count > 209) break;
            }
        });
    }
    for(int i=0;i<10;i++){
        t[i].join();
    }
    return 0;
}
