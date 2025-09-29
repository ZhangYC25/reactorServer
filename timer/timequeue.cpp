#include "timequeue.h"
#include "eventloop.h"
#include "channel.h"
#include "timestamp.h"
#include <memory>
#include <sys/timerfd.h>
#include <cstring>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>

//const int32_t kMicrosecond2Sencond kMicrosecond2Second = 1000 * 1000;

//#define  kMicrosecond2Second 1000*1000

//const int32_t kMicrosecond2Sencond = 1000*1000;

TimeQueue::TimeQueue(EventLoop* loop) : loop_(loop){
    CreateTimerFd();
    printf("log time ch\n");
    channel_ = std::make_unique<Channel>(timerfd_, loop_);
    channel_ -> set_read_callback(std::bind(&TimeQueue::HandleRead, this));
    channel_ -> EnableRead();
}

void TimeQueue::CreateTimerFd(){
    timerfd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd_ < 0) {printf("TimerQueue::CreateTimefd error");}
}

void TimeQueue::ReadTimerFd(){
    uint64_t read_byte;
    ssize_t nread = ::read(timerfd_, &read_byte, sizeof(read_byte));
    if (nread != sizeof(read_byte)) { printf("TimerQueue::ReadTimerFd read error");}
}

TimeQueue::~TimeQueue(){
    loop_ -> DeleteChannel(channel_.get());
    close(timerfd_);
    for (const auto& entry : timers_) {
        delete entry.second;
    }
}

void TimeQueue::AddTimer(TimeStamp timestamp, std::function<void()> const &cb, double interval){
    Timer * timer = new Timer(timestamp, cb, interval);

    if (Insert(timer))
    {

        ResetTimerFd(timer);
    }
}

bool TimeQueue::Insert(Timer * timer){
    bool reset_instantly = false;
    if(timers_.empty() || timer->expiration() < timers_.begin()->first){
        reset_instantly = true;
    }
    timers_.emplace(std::move(Entry(timer->expiration(), timer)));
    if (reset_instantly == true) printf("true\n");
    return reset_instantly;
}

void TimeQueue::HandleRead(){
    ReadTimerFd();
    active_timers_.clear();

    auto end = timers_.lower_bound(Entry(TimeStamp::Now(), reinterpret_cast<Timer*>(UINTPTR_MAX)));
    active_timers_.insert(active_timers_.end(), timers_.begin(),end);

    timers_.erase(timers_.begin(), end);
    for (const auto &entry : active_timers_) {entry.second -> run();}
    ResetTimer();
}

void TimeQueue::ResetTimer(){
    for (auto& entry : active_timers_) {
        if ((entry.second) -> repeat()) {
            auto timer = entry.second;
            timer -> ReStart(TimeStamp::Now());
            Insert(timer);
        } else {
            delete entry.second;
        }
    }
    if (!timers_.empty()) {ResetTimerFd(timers_.begin() -> second);}
}

void TimeQueue::ResetTimerFd(Timer* timer){
    struct itimerspec new_;
    struct itimerspec old_;
    memset(&new_, '\0', sizeof(new_));
    memset(&old_, '\0', sizeof(old_));

    int64_t micro_seconds_dif = timer->expiration().microseconds() - TimeStamp::Now().microseconds();
    if (micro_seconds_dif < 100){
        micro_seconds_dif = 100;
    }

    new_.it_value.tv_sec = static_cast<time_t>(
        micro_seconds_dif / kMicrosecond2Sencond);
    new_.it_value.tv_nsec = static_cast<long>((
        micro_seconds_dif % kMicrosecond2Sencond) * 1000);
    int ret = ::timerfd_settime(timerfd_, 0, &new_, &old_);
    
    assert(ret != -1);
    (void)ret;
}