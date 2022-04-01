#ifndef __ZOS_UDP_SOCKET_H__
#define __ZOS_UDP_SOCKET_H__
#include <string>
#include <array>
#include <asio.hpp>
#include <fmt/core.h>

#include "zos/meta.h"

#include "utils/singleton.h"

namespace zos{
using __io = Singleton<asio::io_context>;
namespace udp{
using __callback_type = zos::meta::socket_callback_type;
using endpoint=asio::ip::udp::endpoint;
using address = asio::ip::address;
class socket{
public:
    socket():_socket(__io::GetInstance(),asio::ip::udp::v4()){}
    socket(const asio::ip::udp::endpoint& ep,const __callback_type& f = {}):socket(ep,nullptr,f){}
    socket(const asio::ip::udp::endpoint& ep,const char* multicast_address,const __callback_type& f = {}):_listen_ep(ep),_socket(__io::GetInstance(),ep.protocol()){
        if(multicast_address!=nullptr && multicast_address!=""){
            _socket.set_option(asio::ip::udp::socket::reuse_address(true));
            _socket.set_option(asio::ip::multicast::join_group(asio::ip::address::from_string(multicast_address)));
        }
        if(f){
            _callback = std::bind(f,std::placeholders::_1,std::placeholders::_2);
        }
        std::error_code ec;
        _socket.bind(_listen_ep,ec);
        std::cout << fmt::format("first output ec {}:{}",ec.value(),ec.message()) << std::endl;
        if(ec.value() != 0){
            std::cerr << fmt::format("get error {}:{}",ec.value(),ec.message()) << std::endl;
        }
        _socket.async_receive_from(asio::buffer(_data,MAX_LENGTH),_received_ep
            , std::bind(&socket::handle_receive_from, this, std::placeholders::_1, std::placeholders::_2)
        );
    }
    ~socket() = default;
    void join_multicast(const char* multicast_address){
        _socket.set_option(asio::ip::udp::socket::reuse_address(true));
        _socket.set_option(asio::ip::multicast::join_group(asio::ip::address::from_string(multicast_address)));
    }
    void set_callback(const __callback_type& f){
        _callback = std::bind(f,std::placeholders::_1,std::placeholders::_2);
    }
    bool try_bind(){
        std::error_code ec;
        _socket.bind(_listen_ep,ec);
        std::cout << fmt::format("first output ec {}:{}",ec.value(),ec.message()) << std::endl;
        if(ec.value() != 0){
            std::cerr << fmt::format("get error {}:{}",ec.value(),ec.message()) << std::endl;
            return false;
        }
        _socket.async_receive_from(asio::buffer(_data,MAX_LENGTH),_received_ep
            , std::bind(&socket::handle_receive_from, this, std::placeholders::_1, std::placeholders::_2)
        );
        return true;
    }
    // use for sender
    void send_to(const std::string& str,const asio::ip::udp::endpoint& endpoint){
        _socket.send_to(asio::buffer(str.c_str(),str.size()),endpoint);
    }
private:
    void handle_receive_from(const std::error_code &ec, size_t bytes_recvd){
        std::cout << fmt::format("output ec {}:{}",ec.value(),ec.message()) << std::endl;
        if (ec.value() == 0){
            if(_callback) std::invoke(_callback,_data.data(),bytes_recvd);
            _socket.async_receive_from(asio::buffer(_data,MAX_LENGTH),_received_ep
                , std::bind(&socket::handle_receive_from, this, std::placeholders::_1, std::placeholders::_2)
            );
        }else{
            std::cerr << fmt::format("get error {}:{}",ec.value(),ec.message()) << std::endl;
        }
    }
private:
    asio::ip::udp::endpoint _listen_ep,_received_ep;
    asio::ip::udp::socket _socket;
    enum{
        MAX_LENGTH = 1024
    };
    std::array<char,MAX_LENGTH> _data;
    __callback_type _callback = {};
};
} // namespace zos::udp
} // namespace zos
#endif // __ZOS_UDP_SOCKET_H__