#pragma once
#include "commom.h"
#include <sys/epoll.h>
#include <functional>
#include <memory>

class EventLoop;
class Socket;
class Channel{
private:
    //Epoll* ep;
    EventLoop* loop_;
    int fd_;
    short listen_events_;
    short ready_events_;
    //bool useThreadpool;
    bool in_epoll_{false};
    std::function<void()> read_callback_;
    std::function<void()> write_callback_;
    bool tied_;
    std::weak_ptr<void> tie_;
public:
    DISALLOW_COPY_AND_MOVE(Channel);
    Channel(int _fd, EventLoop* _loop);
    ~Channel();
    //回调处理
    void HandleEvent() const;
    void HandleEventWithGuard() const;
    //add in epoll
    void EnableRead();
    void EnableWrite();
    void EnableET();
    void DisableWrite();

    int fd() const;
    short listen_events() const;
    short ready_events() const;

    bool IsInEpoll() const;
    void SetInEpoll(bool in = true);
    
    void SetReadyEvents(int ev);
    void set_read_callback(std::function<void()> const& callback);
    void set_write_callback(std::function<void()> const& callback);

    void Tie(const std::shared_ptr<void> & ptr);
};