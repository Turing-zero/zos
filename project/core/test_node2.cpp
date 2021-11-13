#include "zos/zos.h"
#include <string>
#include <chrono>
using namespace std::chrono_literals;
class A:public zos::Node{
public:
    A():zos::Node("A_node"){
        declare_publish("msg");
    }
    void run() override{
        zos::Data buffer;
        int count = 0;
        while(true){
            std::string msg = std::to_string(++count);
            buffer.store(msg.c_str(),msg.size());
            publish("msg",buffer);
            zos::log("after publish {}\n",count);
            std::this_thread::sleep_for( 1ms );
            if(count > 10) break;
        }
    }
};
class B:public zos::Node{
public:
    B():zos::Node("B_node"){
        declare_receive<2>("msg");
    }
    void run() override{
        zos::Data buffer;
        while(true){
            receive("msg",buffer);
            std::string msg(static_cast<const char*>(buffer.data()),buffer.size());
            int count = std::stoi(msg);
            zos::log("got count : {}\n",count);
            std::this_thread::sleep_for( 300ms );
            if(count > 10) break;
        }
    }
};
int main(){
    A a;
    B b;
    a.link(&b,"msg");
    b.start();
    a.start();
    a.join();
    b.join();
    return 0;
}