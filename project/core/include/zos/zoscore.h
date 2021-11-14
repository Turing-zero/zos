#ifndef ZOSCORE_H
#define ZOSCORE_H
#include <string>
#include <map>
#include <list>
#include <tuple>

#include <typeinfo>

#include "zos/token.h"
#include "zos/semadata.h"
#include "zos/log.h"

#include <iostream>
namespace zos{
class NodeManager;
class Node{
public:
    Node(const std::string& name):_name(name){
        #ifdef ZOS_DEBUG
        zos::log("{} {} zos::Node constructor\n",fmt::ptr(this),this->_name);
        #endif
    }
    Node() = delete;
    Node(const Node&) = delete;
    virtual ~Node(){
        for(auto&& it:this->_databox){

        }
        // TODO
        #ifdef ZOS_DEBUG
        zos::log("{} {} zos::Node destructor\n",fmt::ptr(this),this->_name);
        #endif
    }
    const std::string& name() const{ return _name; }
    virtual void run() = 0;
    virtual void start() final{
        _t = std::thread([=,this]{
            run();
            #ifdef ZOS_DEBUG
            zos::log("{} {} has exit.\n",fmt::ptr(this),this->_name);
            #endif
        });
    }
    virtual void join() final{
        _t.join();
    }
    template<unsigned int buffer_size=1>
    void declare_receive(const std::string& msg){
        #ifdef ZOS_DEBUG
        zos::log("{} {} declare_receive {}\n",fmt::ptr(this),this->_name,msg);
        #endif
        std::unique_lock u_lock(this->_mutex_databox);
        auto it = _databox.find(msg);
        if (it != _databox.end()){
            std::cerr << "ERROR : REDECLARE, check your message type : " << msg << std::endl;
            return;
        }
        _databox.insert(std::pair(msg,new SemaData<buffer_size>()));
    }
    virtual void declare_publish(const std::string& msg) final{
        #ifdef ZOS_DEBUG
        zos::log("{} {} declare_publish {}",fmt::ptr(this),this->_name,msg);
        #endif
        std::unique_lock u_lock(this->_mutex_subscriber);
        if (auto it = _subscribers.find(msg);it != _subscribers.end()){
            std::cerr << "ERROR : REDECLARE_PUBLISH, check your message type : " << msg << std::endl;
            return;
        }
        _subscribers[msg] = {};
    }
    virtual void publish(const std::string& msg, const void* data = nullptr, const unsigned long size = 0) final{
        std::shared_lock s_lock(_mutex_subscriber);
        auto it = _subscribers.find(msg);
        if(it != _subscribers.end()){
            for(auto p:_subscribers[msg]){
                p.second->store(data,size);
            }
        }
    }
    virtual void publish(const std::string& msg,const Data& data) final{
        this->publish(msg,data.data(),data.size());
    }
    virtual void receive(const std::string& msg,Data& data) final{
        std::shared_lock s_lock(this->_mutex_databox);
        auto&& it = _databox.find(msg);
        if (it == _databox.end()){
            std::cerr << "ERROR : didn't DECLARE to RECEIVE this kind of message, check your message type : " << msg << std::endl;
            return;
        }
        it->second->pop(data);
    }
    virtual bool try_receive(const std::string& msg, Data& data) final{
        std::shared_lock s_lock(this->_mutex_databox);
        auto &&it = _databox.find(msg);
        if (it == _databox.end())
        {
            std::cerr << "ERROR : didn't DECLARE to RECEIVE this kind of message, check your message type : " << msg << std::endl;
            return false;
        }
        auto res = it->second->try_pop(data);
        return res;
    }
    virtual void link(Node* n,const std::string& msg) final{
        std::unique_lock u_lock(this->_mutex_subscriber);
        std::shared_lock s_lock(n->_mutex_databox);
        auto it = _subscribers.find(msg);
        if (it == _subscribers.end()){
            std::cerr << "ERROR : didn't DECLARE to PUBLISH message : " << msg << std::endl;
            return;
        }
        if (auto iit = it->second.find(n);iit != it->second.end()){
            std::cerr << "ERROR : already LINKed this message : [" << msg << "] before, maybe some error?" << std::endl;
            return;
        }
        auto iit = n->_databox.find(msg);
        if(iit == n->_databox.end()){
            std::cerr << "ERROR : didn't DECLARE to STORE this kind of message, check your message type : " << msg << std::endl;
            return;
        }
        zos::log("link : {} [{}] --> {}\n",this->_name,msg,n->_name);
        it->second.insert(std::pair(n,iit->second));
        #ifdef ZOS_DEBUG
        zos::log(" link success, total subscribers of [{}] in {} : {}\n",msg,this->_name,it->second.size());
        #endif
    }
    virtual void unlink(Node* n,const std::string& msg) final{
        std::unique_lock lock(_mutex_subscriber);
        auto it = _subscribers.find(msg);
        if (it == _subscribers.end()){
            std::cerr << "ERROR : didn't DECLARE to PUBLISH message : " << msg << std::endl;
            return;
        }
        auto iit = it->second.find(n);
        if (iit == it->second.end()){
            std::cerr << "ERROR : didn't LINK this message : " << msg << std::endl;
            return;
        }
        zos::log("unlink : {} [{}] -\\-> {}\n",this->_name,msg,n->_name);
        it->second.erase(iit);
        #ifdef ZOS_DEBUG
        zos::log(" unlink success, total subscribers of [{}] in {} : {}",msg,this->_name,it->second.size());
        #endif
    }

private:
    mutable std::shared_mutex _mutex_subscriber, _mutex_databox;
    std::map<std::string,std::map<Node*,ISemaData*>> _subscribers = {};
    std::map<std::string,ISemaData*> _databox = {};
    std::thread _t;
    std::atomic<Status> _status = Status::PREPARE;
    std::atomic<Token> _token = Token::PREPARE;
    const std::string _name;
};
class NodeManager{};
} // namespace zos

#endif // ZOSCORE_H
