#include "Epoll.h"
#include "util.h"
#include "channel.h"

#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <iostream>

#define MAX_EVENTS 1024


Epoll::Epoll() {
    epfd_ = epoll_create1(0);
    events_ = new epoll_event[MAX_EVENTS];
    memset(events_, 0, sizeof(epoll_event)*MAX_EVENTS);
    errif(epfd_ == -1, "epoll create error");
}

Epoll::~Epoll(){
    if(epfd_ != -1){
        ::close(epfd_);
    }
    delete[] events_;
}

std::vector<Channel*> Epoll::Poll(int timeout) const {
    std::vector<Channel*> active_channels;
    int nfds = epoll_wait(epfd_, events_, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    for (int i = 0; i < nfds; ++i){
        Channel* ch = (Channel*)events_[i].data.ptr;
        int events = events_[i].events;
        ch -> SetReadyEvents(events);
        active_channels.push_back(ch);
    }
    return active_channels;
}

void Epoll::UpdateChannel(Channel* channel) const{
    int fd = channel -> fd();
    struct epoll_event ev;
    memset(&ev, 0, sizeof(struct epoll_event));
    ev.data.ptr = channel;
    ev.events = channel -> listen_events();
    if(!channel->IsInEpoll()){
        errif(epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->SetInEpoll(true);
        // debug("Epoll: add Channel to epoll tree success, the Channel's fd is: ", fd);
    } else{
        errif(epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
        // debug("Epoll: modify Channel in epoll tree success, the Channel's fd is: ", fd);
    }
}

void Epoll::DeleteChannel(Channel *channel) const{
    int fd = channel->fd();
    //printf("移除fd\n");
    errif(epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) == -1, "epoll delete error");
    channel->SetInEpoll(false);
}