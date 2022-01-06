#include <iostream>
#include <string>
#include "zos/socket.h"

void _cb(const void* p,size_t lens){
    std::string s(static_cast<const char*>(p),lens);
    std::cout << "marktest : " << s << std::endl;
}

int main(){
    int port = 30001;
    zos::udp::endpoint listen_ep(zos::udp::address::from_string("0.0.0.0"),30001);
    zos::udp::socket socket(listen_ep,"233.233.233.233",_cb);
    return zos::__io::GetInstance().run();
}