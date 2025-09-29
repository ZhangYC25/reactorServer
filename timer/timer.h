#pragma once
#include <functional>
#include "commom.h"
#include "timestamp.h"
class Timer{
public:
    DISALLOW_COPY_AND_MOVE(Timer);
    Timer(TimeStamp timestamp, std::function<void()>const &cb, double interval);
    void ReStart(TimeStamp now);
    
    void run() const;
    TimeStamp expiration() const;
    bool repeat() const;
   
private:
    TimeStamp expiration_;
    std::function<void()> callback_;
    double interval_;
    bool repeat_;
};