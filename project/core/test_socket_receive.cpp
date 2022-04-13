#include <iostream>
#include <string>
#include "zos/socket.h"
#include "zos/socketplugin.h"
#include "zos.pb.h"
void _cb(const void* p,size_t lens){
    std::string s(static_cast<const char*>(p),lens);
    std::cout << "marktest : " << s << std::endl;
}

int main(){
    int port = 30001;
    zos::udp::Plugin<zos::pb::Msg4Test> udp(port,_cb);
    std::thread t([]{
        std::cout << "before" << std::endl;
        zos::__io::GetInstance()->run();
        std::cout << "after" << std::endl;
    });
    t.join();
    return 0;//zos::__io::GetInstance()->run();
}
