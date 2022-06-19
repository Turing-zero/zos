#ifndef __ZOS_UDP_SOCKET_MANAGER_H__
#define __ZOS_UDP_SOCKET_MANAGER_H__
#include <atomic>
#include "zos/socket.h"
#include "zos.pb.h"
namespace zos{
namespace udp{
class manager:public Singleton<manager>{
    static constexpr const char* default_interface = "0.0.0.0";
    static constexpr const char* default_mc_address = "233.233.233.233";
    static constexpr const int default_port = 23333;
public:
    manager(const char* _if=default_interface,const char* _address=default_mc_address,const int _port=default_port):_endpoint(address::from_string(_address),_port),_interface(_if),_mc_address(_address),_port(_port){
        _write.set_interface(zos::udp::address::from_string(_interface));
        _read.join_multicast(address::from_string(_mc_address),address::from_string(_interface));
        _read.bind(endpoint(address_v4::any(),_port),std::bind(&manager::_cb,this,std::placeholders::_1,std::placeholders::_2));
        _state_send = std::thread([this]{
            int count = 0;
            zos::pb::State _state;
            zos::Data _data;
            while(!need_exit.load()){
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                auto size = _state.ByteSize();
                _data.resize(size);
                _state.SerializeToArray(_data.ptr(),size);
                _write.send_to(_data.data(),_data.size(),_endpoint);
            }
            zos::log("_state_send exit\n");
        });
        _asio_context = std::thread([this]{
            running = true;
            zos::__io::_()->run();
            zos::__io::_()->reset();
            running = false;
            zos::log("_asio_context exit\n");
        });
    }
    ~manager(){
        zos::log("in dtor of manager\n");
        need_exit = true;
        _state_send.join();
        zos::__io::_()->stop();
        _asio_context.join();
    }
    void _cb(const void* p,size_t lens){
        _state.ParseFromArray(p,lens);
        getState(_state);
    }
private:
    void getState(const zos::pb::State& _state){
        zos::log("get state : {}\n",_state.ShortDebugString());
    }
    zos::pb::State _state;
    const std::string _interface;
    const std::string _mc_address;
    const int _port;
    const endpoint _endpoint;

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
