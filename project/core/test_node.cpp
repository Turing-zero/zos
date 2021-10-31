#include "zos/zos.h"
#include <string>
#include <chrono>
using namespace std::chrono_literals;
class A:public zos::Node{
public:
    A(const std::string& name):zos::Node(name){
        declare_publish("msg");
    }
    void run() override{
        while(true){
            std::this_thread::sleep_for(1s);
        }
    }
};
class B:public zos::Node{
public:
    B():zos::Node("B"){
        declare_receive("msg");
    }
    void run() override{
        while(true){
            std::this_thread::sleep_for(1s);
        }
    }
};
int main(){
    A a1("A111"),a2("A222");
    B b;
    B* pb = &b;
    a1.link(&b,"msg");
    a2.link(pb,"msg");
    a1.link(pb,"msg");
    a1.unlink(pb,"msg");
    a1.unlink(pb,"msg");
    return 0;
}