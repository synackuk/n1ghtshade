//
//  Manager.hpp
//  libgeneral
//
//  Created by tihmstar on 17.11.20.
//  Copyright © 2020 tihmstar. All rights reserved.
//

#ifndef Manager_hpp
#define Manager_hpp

#include <future>

namespace tihmstar {
    enum loop_state{
        LOOP_UNINITIALISED = 0,
        LOOP_CONSTRUCTING,
        LOOP_RUNNING,
        LOOP_STOPPING,
        LOOP_STOPPED
    };
    /*
     Abstract class
     */
    class Manager{
        std::thread *_loopThread;
    protected:
        std::atomic<loop_state> _loopState;
        
        virtual void loopEvent();
        
    public:
        Manager(const Manager&) = delete; //delete copy constructor
        Manager(Manager &&o) = delete; //move constructor
        
        Manager(); //default constructor
        virtual ~Manager();
        
        void startLoop();
        void stopLoop() noexcept;

        virtual void beforeLoop(); //execute before Loop started
        virtual void afterLoop() noexcept; //execute after Loop stopped (e.g. because it died)
        virtual void stopAction() noexcept; //execute when stopping Loop (before waiting for the thread to finish)
    };
};


#endif /* Manager_hpp */
