#include <iostream>
#include <string>
#include "zos/socket.h"
#include "zos/socketplugin.h"
#include "zos_test.pb.h"
void _cb(const void* p,size_t lens){
    std::string s(static_cast<const char*>(p),lens);
    std::cout << "marktest : " << s << std::endl;
}

void test(){
    int port = 30001;
    asio::ip::udp::endpoint receiver_endpoint(zos::udp::address_v4::any(),port);

    zos::udp::socket socket;
    socket.join_multicast(zos::udp::address::from_string("233.233.233.233"),zos::udp::address::from_string("10.12.225.58"));
    socket.bind(receiver_endpoint,_cb);

    zos::__io::_()->run();
}
int main(){
    test();
    return 0;
}
