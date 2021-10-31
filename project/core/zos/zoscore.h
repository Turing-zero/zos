#ifndef ZOSCORE_H
#define ZOSCORE_H
#include <string>
#include <map>
#include <list>
#include <tuple>

#include <typeinfo>
#include <iostream>

#include "zos/token.h"
#include "zos/semadata.h"
#include "zos/log.h"

namespace zos{
class NodeManager;
class Node{
public:
    Node(const std::string& name):_name(name){
        #ifdef ZOS_DEBUG
        std::cout << this << ' ' << this->_name << " zos::Node constructor" << std::endl;
        #endif
    }
    Node() = delete;
    Node(const Node&) = delete;
    virtual ~Node(){
        // TODO
        #ifdef ZOS_DEBUG
        std::cout << this << ' ' << this->_name << " zos::Node destructor" << std::endl;
        #endif
    }
    const std::string& name() const{ return _name; }
    virtual void run() = 0;

    template<unsigned int buffer_size=1>
    void declare_receive(const std::string& msg){
        #ifdef ZOS_DEBUG
        std::cout << this << ' ' << this->_name << " declare_receive " << msg << std::endl;
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
        std::unique_lock u_lock(this->_mutex_subscriber);
        std::cout << this << ' ' << this->_name << " declare_publish " << msg << std::endl;
        #endif
        if (auto it = _subscribers.find(msg);it != _subscribers.end()){
            std::cerr << "ERROR : REDECLARE_PUBLISH, check your message type : " << msg << std::endl;
            return;
        }
        _subscribers[msg] = {};
    }
    virtual void publish(const std::string& msg, const void* data = nullptr, const unsigned long size = 0) final{}
    virtual void receive(const std::string& msg,Data& data) final{}
    virtual bool try_receive(const std::string& msg, Data& data) final{}
    virtual void link(Node* n,const std::string& msg) final{
        #ifdef ZOS_DEBUG
        std::cout << this << ' ' << this->_name << " link : " << msg << " to " << n << ' ' << n->_name << std::endl;
        #endif
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
        std::cout << " link success, total subscribers of [" << msg << "] in " << this->_name << " : " << it->second.size() << std::endl;
        #endif
    }
    virtual void unlink(Node* n,const std::string& msg) final{
        #ifdef ZOS_DEBUG
        std::cout << this << ' ' << this->_name << " unlink : [" << msg << "] to " << n << ' ' << n->_name << std::endl;
        #endif
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
        std::cout << " unlink success, total subscribers of [" << msg << "] in " << this->_name << " : " << it->second.size() << std::endl;
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
