#include <iostream>
#include <chrono>
#include <thread>
#include <fmt/core.h>
#include "zos/socket.h"

int main(){
    int count = 0;
    int port = 30001;
    zos::udp::socket socket;
    socket.set_interface(zos::udp::address::from_string("192.168.31.122"));
    // socket.join_multicast("233.233.233.233");

    asio::ip::udp::endpoint receiver_endpoint(zos::udp::address::from_string("233.233.233.233"),port);

    while(true){
        socket.send_to(fmt::format("count : {}",count++),receiver_endpoint);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
