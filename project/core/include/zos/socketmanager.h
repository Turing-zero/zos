#ifndef __ZOS_UDP_SOCKET_MANAGER_H__
#define __ZOS_UDP_SOCKET_MANAGER_H__
#include <atomic>
#include "zos/socket.h"
#include "zos.pb.h"
namespace zos{
namespace udp{
class manager{
    static constexpr const char* default_interface = "0.0.0.0";
    static constexpr const char* default_mc_address = "233.233.233.233";
    static constexpr const int default_port = 23333;
public:
    manager(const char* _if=default_interface,const char* _address=default_mc_address,const int _port=default_port):_interface(_if),_mc_address(_address),_port(_port){
        _write.set_interface(_interface.c_str());
        _state_send = std::thread([this]{
            int count = 0;
            zos::pb::State _state;
            while(!need_exit.load()){
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                zos::log("state_send thread:{}\n",count++);
            }
            zos::log("_state_send exit\n");
        });
        _asio_context = std::thread([this]{
            running = true;
            zos::__io::GetInstance()->run();
            zos::__io::GetInstance()->reset();
            running = false;
            zos::log("_asio_context exit\n");
        });
    }
    ~manager(){
        zos::log("in dtor of manager\n");
        need_exit = true;
        _state_send.join();
        _asio_context.join();
    }
private:
    const std::string _interface;
    const std::string _mc_address;
    const int _port;

    socket _read;
    socket _write;

    std::thread _asio_context;
    std::thread _state_send;
    std::atomic<bool> running = false;
    std::atomic<bool> need_exit = false;
};
} // zos::udp
} // zos
#endif // __ZOS_UDP_SOCKET_MANAGER_H__
