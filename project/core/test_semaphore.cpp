#include "zos/zos.h"
#include <chrono>
using namespace std::chrono_literals;

Semaphore<> s(0);
void pong(){
    static int count=0;
    // while(!_t.stop_requested()){
    while(true){
        if(s.try_acquire_for(10)){
            zos::log("get {} from pong\n",count++);
            std::this_thread::sleep_for(500ms);
        }else{
            zos::log("wait for 1 second... timeout!\n");
            break;
        }
    }
    zos::log("pong stopped\n");
}

int main(int argc, char *argv[])
{
    std::thread t(pong);
    for(int i=0;i<6;i++){
        std::this_thread::sleep_for(300ms);
        zos::log("send {} from main\n",i);
        s.release();
    }
    t.join();
    return 0;
}
