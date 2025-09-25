#include "tcpserver.h"
#include "tcpconnection.h"
#include "eventloop.h"
#include "acceptor.h"
#include "currentthread.h"
#include "eventloop.h"
#include "eventloopthreadpool.h"
#include "commom.h"
#include <unistd.h>
#include <iostream>
// #include <memory>
#include <assert.h>


TcpServer::TcpServer(EventLoop* loop, int port): main_reactor_(loop),next_conn_id_(1){
    //main_reactor_ = std::make_unique<EventLoop>();
    acceptor_ = std::make_unique<Acceptor>(main_reactor_, port);
    std::function<void(int)> cb = std::bind(&TcpServer::HandleNewConnection, this, std::placeholders::_1);
    acceptor_->set_newonnection_callback_(cb);

    //建立从reactor
    //unsigned int size = std::thread::hardware_concurrency();
    thread_pool_ = std::make_unique<EventLoopThreadPool>(loop);
}

TcpServer::~TcpServer(){

};

void TcpServer::Start(){
    thread_pool_ -> Start();

    main_reactor_ -> Loop();
}

void TcpServer::HandleNewConnection(int fd){
    
    assert(fd != -1);
    std::cout << "New connection fd: " << fd << std::endl;
    //uint64_t random = fd % sub_reactors_.size();
    EventLoop* sub_reactor = thread_pool_ -> nextloop();
    std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(sub_reactor,fd,next_conn_id_);
    std::function<void(const std::shared_ptr<TcpConnection> &)> cb = std::bind(&TcpServer::HandleClose, this, std::placeholders::_1);
    conn -> set_connection_callback(on_connect_);
    conn->set_close_callback(cb);
    conn->set_message_callback(on_message_);

    connectionsMap_[fd] = conn;
    // 分配id
    ++next_conn_id_;
    if(next_conn_id_ == 1000){
        next_conn_id_ = 1;
    }
    conn -> ConnectionEstablished();
}

void TcpServer::HandleClose(const std::shared_ptr<TcpConnection> & conn){
    std::cout <<  CurrentThread::tid() << " TcpServer::HandleClose"  << std::endl;
    main_reactor_->RunOneFunc(std::bind(&TcpServer::HandleCloseInLoop, this, conn));
}

inline void TcpServer::HandleCloseInLoop(const std::shared_ptr<TcpConnection> & conn){
    std::cout << CurrentThread::tid()  << " TcpServer::HandleCloseInLoop - Remove connection id: " <<  conn->id() << " and fd: " << conn->fd() << std::endl;
    auto it = connectionsMap_.find(conn->fd());
    assert(it != connectionsMap_.end());
    connectionsMap_.erase(connectionsMap_.find(conn->fd()));

    EventLoop *loop = conn->loop();
    loop->QueueOneFunc(std::bind(&TcpConnection::ConnectionDestructor, conn));
}

void TcpServer::set_connection_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn) { on_connect_ = std::move(fn); };
void TcpServer::set_message_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn) { on_message_ = std::move(fn); };
void TcpServer::SetThreadNums(int thread_nums) {thread_pool_ -> SetThreadNums(thread_nums);}