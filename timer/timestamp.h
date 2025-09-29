#pragma once
#include <stdio.h>
#include <sys/time.h>
#include <string>

const int32_t kMicrosecond2Sencond = 1000 * 1000;

class TimeStamp{
public:
    TimeStamp() : micro_seconds_(0){}
    explicit TimeStamp(int64_t micro_seconds) : micro_seconds_(micro_seconds){}

    bool operator<(const TimeStamp &rhs) const{
        return micro_seconds_ < rhs.microseconds();
    }

    bool operator==(const TimeStamp &rhs) const{
        return micro_seconds_ == rhs.microseconds();
    }
    int64_t microseconds() const{
        return micro_seconds_;
    }

    std::string ToForMattedString() const{
        char buf[64] = {0};
        time_t seconds = static_cast<time_t> (micro_seconds_ / kMicrosecond2Sencond);
        struct tm tm_time;
        localtime_r(&seconds, &tm_time);
        int microseconds = static_cast<int>(micro_seconds_ % kMicrosecond2Sencond);
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
                    tm_time.tm_year+1900, tm_time.tm_mon+1,tm_time.tm_mday,
                    tm_time.tm_hour,tm_time.tm_min, tm_time.tm_sec, microseconds);
        return buf;            
    }

    static TimeStamp Now();
    static TimeStamp AddTime(TimeStamp timestamp, double add_seconds);

private:
    int64_t micro_seconds_;
};

inline TimeStamp TimeStamp::Now(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return TimeStamp(time.tv_sec * kMicrosecond2Sencond + time.tv_usec);
}

inline TimeStamp TimeStamp::AddTime(TimeStamp timestamp, double add_seconds){
    int64_t add_microseconds = static_cast<int64_t>(add_seconds * kMicrosecond2Sencond);
    return TimeStamp(timestamp.microseconds() + add_microseconds);
}