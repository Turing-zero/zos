#include "zos/zos.h"
#include <string>
#include <chrono>
using namespace std::chrono_literals;
class A{
public:
    A(){}
    void run(){
        zos::Data buffer;
        int count = 0;
        while (true){
            std::string msg = std::to_string(++count);
            buffer.store(msg.c_str(), msg.size());
            publish("msg", buffer);
            // zos::log("after publish {}\n", count);

            if(count%2==0){
                msg.append(",this is a string.");
                buffer.store(msg.c_str(), msg.size());
                publish("msg2",buffer);
            }
            if (count < 3)
                continue;
            if (count == 3)
                std::this_thread::sleep_for(1000ms);
            else {
                std::this_thread::sleep_for(1ms);
            }
            if (count > 10)
                break;
        }
    }
};
class B{
public:
    B(){}
    void run(){
        std::this_thread::sleep_for(300ms);
        zos::Data buffer;
        zos::Data buffer2;
        while (true){
            receive("msg", buffer);
            auto res = try_receive("msg2",buffer2);
            std::string msg(static_cast<const char *>(buffer.data()), buffer.size());
            int count = std::stoi(msg);
            std::string msg2(static_cast<const char *>(buffer2.data()), buffer2.size());
            if(res)
                zos::log("got count : {} and msg : {}\n", count, msg2);
            else
                zos::log("got count : {}, no msg2\n", count);
            
            std::this_thread::sleep_for(300ms);
            if (count > 10)
                break;
        }
    }
};
class C{
public:
    C(){
    }
    void run(){
        std::this_thread::sleep_for(300ms);
        zos::Data buffer;
        zos::Data buffer2;
        while (true){
            receive("msg", buffer);
            auto res = try_receive("msg2",buffer2);
            std::string msg(static_cast<const char *>(buffer.data()), buffer.size());
            int count = std::stoi(msg);
            std::string msg2(static_cast<const char *>(buffer2.data()), buffer2.size());
            if(res)
                zos::log("C got count : {} and msg : {}\n", count, msg2);
            else
                zos::log("C got count : {}, no msg2\n", count);
            if (count > 10)
                break;
        }
    }
};
int main()
{
    A a;
    B b;
    C c;
    a.link("msg",&b,&c);
    a.link("msg2",&b,&c);
    c.start();
    b.start();
    a.start();
    a.join();
    b.join();
    c.join();
    return 0;
}