#ifndef __ZOS_SEMADATA_H__
#define __ZOS_SEMADATA_H__
#include <cstdlib>
#include <cstring>
#include <thread>
#include <limits>
#include <mutex>
#include <shared_mutex>

#include "zos/config.h"
#include "zos/interface.h"
#include "zos/log.h"

#ifndef ZOS_USE_CUSTOM_SEMAPHORE
#include <semaphore>
using Semaphore = std::counting_semaphore;
#else
#include "zos/semaphore.h"
template<unsigned int c=std::numeric_limits<unsigned int>::max()>
using Semaphore = zos::SemaphoreT<c>;

#endif // ZOS_USE_DEFAULT_SEMAPHORE

namespace zos{

// thread unsafe base node
class DataNode{
public:
    DataNode(DataNode* _last=nullptr,DataNode* _next=nullptr):_last(_last),_next(_next),_data(nullptr),_capacity(0),_size(0){
        #ifdef ZOS_DEBUG
        zos::log("{} ZOS DataNode constructor\n",fmt::ptr(this));
        #endif
    }
    virtual ~DataNode(){
        #ifdef ZOS_DEBUG
        zos::log("{} ZOS Data destructor\n",fmt::ptr(this));
        #endif
        if(_capacity > 0){
            free(_data);
        }
    }
    virtual void resize(const unsigned long size){
        if(size > _capacity){
            if(_data != nullptr){
                free(_data);
            }
            _data = malloc(size);
            _capacity = size;
        }
        _size = size;
    }
    DataNode* _last;
    DataNode* _next;
    void* _data;
    unsigned long _capacity;
    unsigned long _size;
};
class Data:public IData,public DataNode{
public:
    Data():DataNode(){
        #ifdef ZOS_DEBUG
        zos::log("{} ZOS Data constructor\n",fmt::ptr(this));
        #endif
    }
    Data(const Data& data){
        #ifdef ZOS_DEBUG
        zos::log("{} ZOS Data copy constructor\n",fmt::ptr(this));
        #endif
        store(data);
    }
    virtual ~Data(){
        #ifdef ZOS_DEBUG
        zos::log("{} ZOS Data destructor\n",fmt::ptr(this));
        #endif
    }
    // self thread-safe
    virtual int size() const {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _size;
    }
    virtual void pop(Data& p) override{
        std::unique_lock<std::shared_mutex> lock(_mutex);
        p.store(this->data(),this->_size);
        resize(0);
    }
    virtual void copyTo(Data* p){
        std::shared_lock<std::shared_mutex> lock(_mutex);
        p->store(this->data(),this->_size);
    }
    virtual void store(const Data& data){
        store(data.data(),data._size);
    }
    virtual void store(const Data* data){
        store(data->data(),data->_size);
    }
    virtual void store(const void* const data,unsigned long size) override{
        std::unique_lock<std::shared_mutex> lock(_mutex);
        resize(size);
        if(size > 0)
            memcpy(_data,data,size);
    }
    // thread-unsafe
    virtual const void* data() const { return _data; }
    virtual void* ptr() {
        return _data;
    }
protected:
    mutable std::shared_mutex _mutex;
};

template<unsigned int _max_capacity=std::numeric_limits<unsigned int>::max()>
class DataQueue:public IData{
public:
    DataQueue():_size(0),_capacity(1),_start(new DataNode()),_end(_start){
        static_assert(_max_capacity>0,"ZOS:max_capacity of DataQueue should be positive.");
        #ifdef ZOS_DEBUG
        zos::log("{} ZOS DataQueue constructor\n",fmt::ptr(this));
        #endif
        _start->_last = _start;
        _start->_next = _start;
    }
    virtual ~DataQueue(){
        #ifdef ZOS_DEBUG
        zos::log("{} ZOS DataQueue destructor\n",fmt::ptr(this));
        #endif
        while(_capacity>1){
            _start = _start->_next;
            delete _start->_last;
            _capacity--;
        }
        delete _start;
    }
    virtual void pop(Data& p) override{
        std::unique_lock<std::shared_mutex> lock(_mutex);
        p.store(_start->_data,_start->_size);
        _start = _start->_next;
        _size--;
        #ifdef ZOS_DEBUG
            zos::log("{} ZOS DataQueue finish pop() ,size={},capa={}\n",fmt::ptr(this),_size,_capacity);
            if(_size < 0){
                zos::log("{} size < 0 after pop({}). _size={}, _capacity={}\n",fmt::ptr(this),fmt::ptr(&p),_size,_capacity);
            }
        #endif
    }
    virtual void store(const Data& data){
        store(data.data(),data._size);
    }
    virtual void store(const Data* data){
        store(data->data(),data->_size);
    }
    virtual void store(const void* const data,unsigned long size) override{
        std::unique_lock<std::shared_mutex> lock(_mutex);
        DataNode* storeNode = nullptr;
        if(_size == _max_capacity){
            storeNode = _end;
            _end = _end->_next;
            _start = _start->_next;
            _size--;
        }else if(_size < _capacity){
            storeNode = _end;
            _end = _end->_next;
        }else{
            storeNode = new DataNode(_end->_last,_end);
            _end->_last->_next = storeNode;
            _end->_last = storeNode;
            _capacity++;
        }
        storeNode->resize(size);
        if(size > 0)
            memcpy(storeNode->_data,data,size);
        _size++;
    }
    virtual unsigned long size(){
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _size;
    }
protected:
    unsigned int _size;
    unsigned int _capacity;
    DataNode* _start;
    DataNode* _end;
    mutable std::shared_mutex _mutex;
};

template<unsigned int max_capacity=std::numeric_limits<unsigned int>::max()>
class SemaData:public ISemaData{
public:
    SemaData():_semaphore(0){
        #ifdef ZOS_DEBUG
        zos::log("{} ZOS SemaData constructor. capa : {}\n",fmt::ptr(this),max_capacity);
        #endif
    };
    virtual ~SemaData(){
        #ifdef ZOS_DEBUG
        zos::log("{} ZOS SemaData destructor.\n",fmt::ptr(this));
        #endif
    };
//    SemaData(const SemaData&) = delete;
    virtual void pop(Data& p) override{
        _semaphore.acquire();
        _data.pop(p);
    }
    virtual void store(const void* const data,unsigned long size) override{
        _data.store(data,size);
        _semaphore.release();
    }
    virtual void release() override{
        _semaphore.release();
    }
    virtual void acquire() override{
        _semaphore.acquire();
    }
    virtual bool try_acquire() override{
        return _semaphore.try_acquire();
    }
    virtual bool try_acquire_for(unsigned int _dur=1) override{
        return _semaphore.try_acquire_for(_dur * __timestep);
    }
    virtual bool try_pop(Data& p) override{
        auto res = _semaphore.try_acquire();
        if (res) {
            _data.pop(p);
        }
        return res;
    }
    virtual bool try_pop_for(Data& p, unsigned int _dur) override{
        auto res = _semaphore.try_acquire_for(_dur * __timestep);
        if (res) {
            _data.pop(p);
        }
        return res;
    }
    //    template<class Rep, class Period>
    //    bool try_acquire_for( const std::chrono::duration<Rep, Period>& rel_time ){
    //        return _semaphore.try_acquire_for(rel_time);
    //    }
private:
    DataQueue<max_capacity> _data;
    Semaphore<max_capacity> _semaphore;
};
} // namespace zos;
#endif // __ZOS_SEMADATA_H__
