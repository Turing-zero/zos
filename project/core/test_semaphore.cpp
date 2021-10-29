#include "zos/zos.h"
#include <fmt/core.h>
#include <string_view>
#include <experimental/source_location>
using std::experimental::source_location;

#include <chrono>
using namespace std::chrono_literals;
using zos::Node;

class A:public Node{

};


template<typename... Ts>
struct log{
    log(std::string_view s,Ts&&... ts,const source_location& lo=source_location::current()){
        fmt::print("{}({}:{})`{}`:",lo.file_name(),lo.line(),lo.column(),lo.function_name());
        fmt::print(s,ts...);
    }
};
template<typename... Ts> log(std::string_view s,Ts&&... ts) -> log<Ts...>;

Semaphore<> s(0);
void pong(){
    static int count=0;
    // while(!_t.stop_requested()){
    while(true){
        if(s.try_acquire_for(10)){
            log("get {} from pong\n",count++);
            std::this_thread::sleep_for(500ms);
        }else{
            log("wait for 1 second... timeout!\n");
            break;
        }

    }
    log("pong stopped\n");
}

int main(int argc, char *argv[])
{
    std::thread t(pong);
    for(int i=0;i<6;i++){
        std::this_thread::sleep_for(300ms);
        log("send {} from main\n",i);
        s.release();
    }
    t.join();
    return 0;
}
