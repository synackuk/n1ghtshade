//
//  lck_container.hpp
//  libgeneral
//
//  Created by tihmstar on 17.11.20.
//  Copyright © 2020 tihmstar. All rights reserved.
//

#ifndef lck_container_h
#define lck_container_h

#include <vector>
#include <memory>
#include <sched.h>
#include <atomic>
#include <mutex>
#include <libgeneral/Event.hpp>

namespace tihmstar {
    template <class Container>
    class lck_contrainer{
        static constexpr const uint32_t maxMembers = 0x10000;
        std::atomic<uint32_t> _members;
        Event _enterEvent;
        Event _leaveEvent;
        Event _notifyEvent;

    public:
        Container _elems;

        inline lck_contrainer();
        inline ~lck_contrainer();

        //readonly access
        inline void addMember();
        inline void delMember();
        
        //write/modify access
        inline void lockMember();
        inline void unlockMember();

        //block until someone removed members
        inline void notifyBlock();
    };

#pragma mark implementation

    template <class Container>
    lck_contrainer<Container>::lck_contrainer()
    : _members(0)
    {
        //
    }

    template <class Container>
    lck_contrainer<Container>::~lck_contrainer(){
        while (_members) {
            _enterEvent.wait();
        }
    }

    template <class Container>
    void lck_contrainer<Container>::addMember(){
        while (true){
            if (_members.fetch_add(1) >= lck_contrainer::maxMembers){
                _members.fetch_sub(1);
                while (_members>=lck_contrainer::maxMembers)
                    _enterEvent.wait();
            }else{
                break;
            }
        }
    }

    template <class Container>
    void lck_contrainer<Container>::delMember(){
        _members.fetch_sub(1);
        _enterEvent.notifyAll();
        _leaveEvent.notifyAll();
        _notifyEvent.notifyAll();
    }

    template <class Container>
    void lck_contrainer<Container>::lockMember(){
        while (true){
            if (_members.fetch_add(lck_contrainer::maxMembers) >= lck_contrainer::maxMembers){
                _members.fetch_sub(lck_contrainer::maxMembers);
                while (_members>=lck_contrainer::maxMembers)
                    _enterEvent.wait();
            }else{
                while (_members > lck_contrainer::maxMembers)
                    _leaveEvent.wait(); //wait until all members are gone
                break;
            }
        }
    }

    template <class Container>
    void lck_contrainer<Container>::unlockMember(){
        _members.fetch_sub(lck_contrainer::maxMembers);
        _enterEvent.notifyAll();
        _leaveEvent.notifyAll();
        _notifyEvent.notifyAll();
    }

    template <class Container>
    void lck_contrainer<Container>::notifyBlock(){
        _notifyEvent.wait();
    }
};


#endif /* lck_container_h */
