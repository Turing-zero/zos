#include <iostream>
#include "zos/log.h"
#include "zos/socketmanager.h"
int main(){
    zos::manager::_();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    zos::log("end of main func\n");
    return 0;
}
