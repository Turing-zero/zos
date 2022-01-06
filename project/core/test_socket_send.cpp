#include <iostream>
#include <chrono>
#include <thread>
#include <fmt/core.h>
#include "zos/socket.h"

int main(){
    int count = 0;
    int port = 30001;
    zos::udp::socket socket;
    socket.join_multicast("233.233.233.233");

    asio::ip::udp::endpoint receiver_endpoint(asio::ip::address::from_string("233.233.233.233"),port);

    while(true){
        socket.send_to(fmt::format("count : {}",count++),receiver_endpoint);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
