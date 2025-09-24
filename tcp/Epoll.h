#pragma once
#include "commom.h"
#include <sys/epoll.h>
#include <vector>

class Channel;
class Epoll{
private:
    int epfd_;
    struct epoll_event* events_;

public:
    DISALLOW_COPY_AND_MOVE(Epoll);
    Epoll();
    ~Epoll();

    //void addFd(int fd, uint32_t op);
    void UpdateChannel(Channel* ch) const;
    void DeleteChannel(Channel* ch) const;
    std::vector<Channel*> Poll(int timeout = -1) const;
};