#ifndef __ZOS_UDP_SOCKET_MANAGER_H__
#define __ZOS_UDP_SOCKET_MANAGER_H__
#include <atomic>
#include "zos/socket.h"
#include "zos/core.h"
#include "zos.pb.h"
namespace zos{
class SPublisher;
class SSubscriber;
class manager:public Singleton<manager>{
    static constexpr const char* default_interface = "0.0.0.0";
    static constexpr const char* default_mc_address = "233.233.233.233";
    static constexpr const int default_port = 23333;
public:
    manager(const char* _if=default_interface,const char* _address=default_mc_address,const int _port=default_port):_endpoint(udp::address::from_string(_address),_port),_interface(_if),_mc_address(_address),_port(_port){
        _write.set_interface(zos::udp::address::from_string(_interface));
        _read.join_multicast(udp::address::from_string(_mc_address),udp::address::from_string(_interface));
        _read.bind(udp::endpoint(udp::address_v4::any(),_port),std::bind(&manager::_cb,this,std::placeholders::_1,std::placeholders::_2));
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
    const udp::endpoint _endpoint;

    udp::socket _read;
    udp::socket _write;

    std::thread _asio_context;
    std::thread _state_send;
    std::atomic<bool> running = false;
    std::atomic<bool> need_exit = false;
};
// SocketPublisher
class SPublisher{
public:
    SPublisher(const std::string& msg):_s(msg,std::bind(&SPublisher::_cb,this,std::placeholders::_1)){}
    void _cb(const zos::Data& data){
        for(auto ep : _subscribers){
            _sender.send_to(data.data(),data.size(),ep);
        }
    }
    template<typename... Ts>
    requires zos::concepts::are_convertiable<udp::endpoint,Ts...>
    void link(Ts... subs){
        std::unique_lock u_lock(this->_mutex_subscriber);
        _subscribers.insert({static_cast<udp::endpoint>(subs)...});
    }
    template<typename... Ts>
    requires zos::concepts::are_convertiable<const udp::endpoint&,Ts...>
    void unlink(Ts... subs){
        std::unique_lock u_lock(this->_mutex_subscriber);
        _subscribers.erase({static_cast<udp::endpoint>(subs)...});
    }

    std::set<udp::endpoint> _subscribers = {};
    mutable std::shared_mutex _mutex_subscriber;
    udp::socket _sender;
    Subscriber<10> _s;
};
// SocketSubscriber
class SSubscriber{
public:
    SSubscriber(const std::string& msg,const udp::endpoint& _ep):_p(msg){
        _receiver.bind(_ep,std::bind(&SSubscriber::_cb,this,std::placeholders::_1,std::placeholders::_2));
    }
    void _cb(const void* data,size_t size){
        _data.store(data,size);
        _p.publish(_data);
    }
    zos::Data _data;
    udp::socket _receiver;
    Publisher _p;
};
} // zos
#endif // __ZOS_UDP_SOCKET_MANAGER_H__
