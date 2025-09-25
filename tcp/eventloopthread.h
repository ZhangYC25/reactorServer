#pragma once
#include "../base/commom.h"

#include <mutex>
#include <condition_variable>
#include <thread>


class EventLoop;
class EventLoopThread{
public:
    DISALLOW_COPY_AND_MOVE(EventLoopThread);
    
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* StartLoop();
private:
    void ThreadCreate();

    EventLoop* loop_;
    std::thread thread_;

    std::mutex mutex_;
    std::condition_variable cv_;
};