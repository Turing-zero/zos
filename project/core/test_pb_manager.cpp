#include <iostream>
#include "zos/log.h"
#include "zos/socketmanager.h"
int main(){
    zos::log("test\n");
    zos::udp::manager manager("127.0.0.1");
    std::this_thread::sleep_for(std::chrono::seconds(4));
    zos::log("test2\n");
    return 0;
}