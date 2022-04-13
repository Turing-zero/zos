#ifndef ZOSCORE_H
#define ZOSCORE_H
#include <string>
#include <map>
#include <list>
#include <tuple>

#include <typeinfo>

#include <iostream>

#include "zos/utils/threadpool.h"
#include "zos/utils/singleton.h"
#include "zos/config.h"
#include "zos/token.h"
#include "zos/semadata.h"
#include "zos/log.h"
#include "zos/meta.h"
#include "zos/socket.h"

namespace zos{
class SubscriberBase;
class Publisher;
class Manager{
public:
    Manager():pool(zos::config::threadpool_nums){}
private:
    ThreadPool pool;
    friend SubscriberBase;
};
using manager = Singleton<Manager>;
class SubscriberBase{
protected:
    using __callback_type = zos::meta::callback_type;
public:
    SubscriberBase(const std::string& msg,const __callback_type& f = {}):_msg(msg),_data(nullptr){
        if(f){
            _callback = std::bind(f,std::placeholders::_1);
        }
    }
    // should be used when there's no callback functions
    void get(Data& data){
        _data->pop(data);
    }
    void get(Data* data=nullptr){
        _data->pop(data);
    }
    bool try_get(Data& data){
        auto res = _data->try_pop(data);
        return res;
    }
protected:
    bool add_task(){
        if(_callback){
            zos::Data data;
            auto res = try_get(data);
            #ifdef ZOS_PLUGIN_DEBUG
            zos::log("add_task data:{},s:{}\n",fmt::ptr(&data),data.size());
            #endif
            if(!res){
                zos::log("error on function add_task, no data received.\n");
                return false;
            }
            manager::GetInstance()->pool.enqueue(_callback,std::move(data));
        }
        return true;
    }
    std::unique_ptr<ISemaData> _data;
    __callback_type _callback = {};
    const std::string _msg;
    friend class Publisher;
};
template<unsigned int buffer_size=2>
class Subscriber:public SubscriberBase{
public:
    Subscriber(const std::string& msg,const __callback_type& f = {}):SubscriberBase(msg,f){
        _data = std::make_unique<SemaData<buffer_size>>();
    }
};
class Publisher{
public:
    Publisher(const std::string& msg):_msg(msg){}
    void publish(const void* data = nullptr, const unsigned long size = 0){
        #ifdef ZOS_PLUGIN_DEBUG
        zos::log("trigger publish -> receiver nums:{}\n",_subscribers.size());
        #endif
        std::shared_lock s_lock(_mutex_subscriber);
        for(auto s:_subscribers){
            s->_data->store(data,size);
            s->add_task();
        }
    }
    void publish(const Data& data){
        this->publish(data.data(),data.size());
    }
    template<typename... Ts>
    requires zos::concepts::are_convertiable<SubscriberBase*,Ts...>
    void link(Ts... subs){
        std::unique_lock u_lock(this->_mutex_subscriber);
        _subscribers.insert(_subscribers.end(),{static_cast<SubscriberBase*>(subs)...});
        #ifdef ZOS_PLUGIN_DEBUG
        zos::log("Publisher link() : ",fmt::ptr(static_cast<SubscriberBase*>(subs))...);
        #endif
    }
private:
    std::vector<SubscriberBase*> _subscribers = {};
    mutable std::shared_mutex _mutex_subscriber;
    const std::string _msg;
};
// class NodeManager{
// public:
//     NodeManager(){
//         _receiver.join_multicast(multicast_address);
//         _receiver.set_callback(std::bind(&NodeManager::_cb,this,std::placeholders::_1,std::placeholders::_2));
//     }
// private:
//     void _cb(const void* p,size_t lens){
//         std::string s(static_cast<const char*>(p),lens);
//         std::cout << "marktest : " << s << std::endl;
//     }
//     zos::udp::socket _sender;
//     zos::udp::socket _receiver;
//     static constexpr char multicast_address[]="233.233.233.233";
//     static constexpr int multicast_port=23233;
// };

} // namespace zos

#endif // ZOSCORE_H
