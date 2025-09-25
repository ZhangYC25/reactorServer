#include "eventloopthreadpool.h"
#include "eventloopthread.h"
#include <memory>
EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop) : 
main_loop_(loop), thread_nums_(0), next_(0){}
EventLoopThreadPool::~EventLoopThreadPool(){}

void EventLoopThreadPool::SetThreadNums(int thread_nums){
    thread_nums_ = thread_nums;
}

void EventLoopThreadPool::Start(){
    for (int i = 0; i < thread_nums_; ++i) {
        std::unique_ptr<EventLoopThread> ptr = std::make_unique<EventLoopThread>();
        threads_.push_back(std::move(ptr));
        loops_.push_back(threads_.back() -> StartLoop());
    }
}

EventLoop* EventLoopThreadPool::nextloop(){
    EventLoop* ret = main_loop_;
    if (!loops_.empty()) {
        ret = loops_[next_ + 1];
        if (next_ == static_cast<int>(loops_.size())) {
            next_ = 0;
        }
    }
    return ret;
}