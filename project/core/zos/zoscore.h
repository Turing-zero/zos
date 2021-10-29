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

namespace zos{
class NodeManager;
class Node{
public:
    Node(const std::string& name):_name(name){}
    template<unsigned int buffer_size=1>
    void declare_receive(const std::string& msg){
        #ifdef ZOS_DEBUG
        std::cout << this << ' ' << this->_name << " declare_receive " << msg << std::endl;
        #endif
        auto it = _databox.find(msg);
        if (it != _databox.end()){
            std::cerr << "ERROR : REDECLARE, check your message type : " << msg << std::endl;
            return;
        }
//        _databox.insert(std::pair(msg,new SemaData<buffer_size>()));
    }
private:
    std::map<std::string,std::list<std::tuple<Node*,ISemaData*>>> _subscribers = {};
    std::map<std::string,ISemaData*> _databox = {};
    std::thread _t;
    std::string _name;
    std::atomic<Status> _status = Status::PREPARE;
    std::atomic<Token> _token = Token::PREPARE;
};
class NodeManager{};
} // namespace zos

#endif // ZOSCORE_H
