#pragma once
#include "commom.h"
#include "eventloop.h"
#include "timer.h"
#include "channel.h"
#include <functional>
#include <memory>
#include <set>
#include <vector>

class TimeQueue{
public:
    DISALLOW_COPY_AND_MOVE(TimeQueue);
    TimeQueue(EventLoop* loop);
    ~TimeQueue();

    void CreateTimerFd();
    void ReadTimerFd();
    void HandleRead();

    void ResetTimerFd(Timer* timer);
    void ResetTimer();

    bool Insert(Timer*);
    void AddTimer(TimeStamp timestamp, std::function<void()> const& cb, double interval);
private:
    typedef std::pair<TimeStamp, Timer*> Entry;
    EventLoop* loop_;
    int timerfd_;
    std::unique_ptr<Channel> channel_;

    std::set<Entry> timers_;
    std::vector<Entry> active_timers_;
};