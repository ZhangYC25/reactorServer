#include "eventloopthread.h"
#include "eventloop.h"

EventLoopThread::EventLoopThread() : loop_(nullptr){}

EventLoopThread::~EventLoopThread(){}

EventLoop* EventLoopThread::StartLoop(){
    thread_ = std::thread(std::bind(&EventLoopThread::ThreadCreate,this));
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cv_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::ThreadCreate(){
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cv_.notify_one();
    }
    loop_ -> Loop();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = nullptr;
    }
}