#pragma once
#include "commom.h"

#include <memory>
#include <functional>
class EventLoop;
class Channel;
class Acceptor{
private:
    EventLoop* loop;
    int listenfd_;
    std::unique_ptr<Channel> channel_;
    std::function<void(int)> new_connection_callback_;
public:
    DISALLOW_COPY_AND_MOVE(Acceptor);
    Acceptor(EventLoop*, const int port);
    ~Acceptor();
    void set_newonnection_callback_(std::function<void(int)> const& callback);

    void Create();
    void Bind(const int port);
    void Listen();
    void AcceptConnection();
};