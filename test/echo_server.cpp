#include "../tcp/acceptor.h"
#include "../tcp/eventloop.h"
#include "../tcp/tcpserver.h"
#include "../tcp/buffer.h"
#include "../tcp/eventloopthreadpool.h"
#include "../tcp/tcpconnection.h"
#include "../base/currentthread.h"
#include <iostream>
#include <functional>
#include <arpa/inet.h>
#include <vector>

class EchoServer{
    public:
        EchoServer(EventLoop *loop, const int port);
        ~EchoServer();

        void start();
        void setThreadNums(int n);
        void onConnection(const std::shared_ptr<TcpConnection> & conn);
        void onMessage(const std::shared_ptr<TcpConnection> & conn);

    private:
        TcpServer server_;
};

EchoServer::EchoServer(EventLoop *loop, const int port) : server_(loop, port){
    server_.set_connection_callback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
    server_.set_message_callback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1));
};
EchoServer::~EchoServer(){};

void EchoServer::start(){
    server_.Start();
}

void EchoServer::setThreadNums(int n){
    server_.SetThreadNums(n);
}

void EchoServer::onConnection(const std::shared_ptr<TcpConnection> & conn){
    // 获取接收连接的Ip地址和port端口
    int clnt_fd = conn->fd();
    struct sockaddr_in peeraddr;
    socklen_t peer_addrlength = sizeof(peeraddr);
    getpeername(clnt_fd, (struct sockaddr *)&peeraddr, &peer_addrlength);

    std::cout << CurrentThread::tid()
              << " EchoServer::OnNewConnection : new connection "
              << "[fd#" << clnt_fd << "]"
              << " from " << inet_ntoa(peeraddr.sin_addr) << ":" << ntohs(peeraddr.sin_port)
              << std::endl;
};

void EchoServer::onMessage(const std::shared_ptr<TcpConnection> & conn){
    // std::cout << CurrentThread::tid() << " EchoServer::onMessage" << std::endl;
    if (conn->state() == TcpConnection::ConnectionState::Connected)
    {
        std::cout << CurrentThread::tid() << "Message from clent " << conn->read_buf()->c_str() << std::endl;
        conn->Send(conn->read_buf()->c_str());
        //conn->HandleClose();
    }
}

int main(int argc, char *argv[]){
    int port;
    if (argc <= 1)
    {
        port = 2048;
    }else if (argc == 2){
        port = atoi(argv[1]);
    }else{
        printf("error");
        exit(0);
    }
    unsigned int size = std::thread::hardware_concurrency();
    EventLoop *loop = new EventLoop();
    EchoServer *server = new EchoServer(loop, port);
    server -> setThreadNums(size);
    server->start();
    
    // delete loop;
    // delete server;
    return 0;
}