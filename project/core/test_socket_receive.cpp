#include <iostream>
#include <string>
#include "zos/socket.h"
#include "zos/socketplugin.h"
#include "zos_test.pb.h"
void _cb(const void* p,size_t lens){
    std::string s(static_cast<const char*>(p),lens);
    std::cout << "marktest : " << s << std::endl;
}
void test1(){
    int port = 30001;
    asio::ip::udp::endpoint receiver_endpoint(asio::ip::address::from_string("0.0.0.0"),port);

    zos::udp::Plugin<zos::pb::Msg4Test> udp(receiver_endpoint,"233.233.233.233",_cb);
    // std::thread t([]{
    //     std::cout << "before" << std::endl;
    //     zos::__io::GetInstance()->run();
    //     std::cout << "after" << std::endl;
    // });
    // t.join();
    zos::__io::GetInstance()->run();
}
void test2(){
    int port = 30001;
    asio::ip::udp::endpoint receiver_endpoint(asio::ip::address::from_string("0.0.0.0"),port);

    zos::udp::socket socket(receiver_endpoint,"233.233.233.233",_cb);
    // zos::udp::socket socket(receiver_endpoint,_cb);

    zos::__io::_()->run();
}
int main(){
    // test1();
    test2();
    return 0;
}
