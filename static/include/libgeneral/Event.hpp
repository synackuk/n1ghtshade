//
//  Event.hpp
//  libgeneral
//
//  Created by tihmstar on 15.08.20.
//  Copyright © 2020 tihmstar. All rights reserved.
//

#ifndef Event_hpp
#define Event_hpp

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <stdint.h>

namespace tihmstar {
    class Event{
        std::mutex _m;
        std::condition_variable _cv;
        std::condition_variable _cm;
        std::atomic<uint64_t> _members;
        uint64_t _curSendEvent;
        uint64_t _curWaitEvent;

    public:
        Event();
        ~Event();
        
        void wait();
        void notifyAll();
        uint64_t members() const;
    };
};

#endif /* Event_hpp */
