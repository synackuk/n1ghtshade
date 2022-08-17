//
//  DeliveryEvent.hpp
//  libgeneral
//
//  Created by tihmstar on 02.11.20.
//

#ifndef DeliveryEvent_hpp
#define DeliveryEvent_hpp

#include <libgeneral/macros.h>
#include <libgeneral/Event.hpp>
#include <queue>

namespace tihmstar {
    template <typename T>
    class DeliveryEvent{
        std::atomic_bool _isDying;
        std::atomic_bool _isFinished;
        std::atomic<uint64_t> _members;
        Event _membersUpdateEvent;
        
        Event _dataWait;
        std::mutex _dataLock;
        std::queue<T> _dataQueue;

    public:
        DeliveryEvent();
        ~DeliveryEvent();
        
        T wait();
        void post(T data);
        /*
            Signalizes that no more data will be appended to this queue.
            After the last element was dequed, kill() will be called.
            further calls to post() are not allowed after this call.
         */
        void finish();
        
        /*
            Releases waiter and hints destruction of the object in near future
            Further calls to wait() or post() are not allowed after calling kill()
         */
        void kill();
    };

#pragma mark implementation

    template <class T>
    DeliveryEvent<T>::DeliveryEvent()
    : _isDying(false),_isFinished(false),_members(0)
    {
        //
    }

    template <class T>
    DeliveryEvent<T>::~DeliveryEvent(){
        _isDying = true;
        while ((uint64_t)_members > 0) {
            _dataWait.notifyAll(); //release waiter
            _membersUpdateEvent.wait();
        }
    }

    template <class T>
    T DeliveryEvent<T>::wait(){
        assure(!_isDying);
        ++_members;
        cleanup([&]{
            --_members;
            _membersUpdateEvent.notifyAll();
        });

        std::unique_lock<std::mutex> ul(_dataLock);
        while (!_dataQueue.size()) {
            retassure(!_isDying && !_isFinished, "object died while waiting on it");
            ul.unlock();
            _dataWait.wait();

            ul.lock();
        }
        T mydata = _dataQueue.front(); _dataQueue.pop();
        if (_isFinished && !_dataQueue.size()) {
            _isDying = true;
        }
        _dataWait.notifyAll();
        return mydata;
    }

    template <class T>
    void DeliveryEvent<T>::post(T data){
        assure(!_isDying && !_isFinished);
        ++_members;
        cleanup([&]{
            --_members;
            _membersUpdateEvent.notifyAll();
        });
        
        _dataLock.lock();
        _dataQueue.push(data);
        _dataWait.notifyAll();
        _dataLock.unlock();
    }

    template <class T>
    void DeliveryEvent<T>::finish(){
        assure(!_isDying);
        ++_members;
        cleanup([&]{
            --_members;
            _membersUpdateEvent.notifyAll();
        });
        _dataLock.lock();
        _isFinished = true;
        if (!_dataQueue.size()) _isDying = true;
        _dataWait.notifyAll();
        _dataLock.unlock();
    }

    template <class T>
    void DeliveryEvent<T>::kill(){
        assure(!_isDying);
        ++_members;
        cleanup([&]{
            --_members;
            _membersUpdateEvent.notifyAll();
        });
        _isDying = true;
        _dataLock.lock();
        _dataWait.notifyAll();
        _dataLock.unlock();
    }
};



#endif /* DeliveryEvent_hpp */
