#include "tcpconnection.h"
#include "channel.h"
#include "buffer.h"
#include "logging.h"
#include "util.h"
#include "eventloop.h"
#include "httpcontext.h"
#include "commom.h"

#include <memory>
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <utility>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>

#define READ_BUFFER 1024
//accept 之后再建立 connection
TcpConnection::TcpConnection(EventLoop* _loop, int connfd, int connid) : loop_(_loop), connfd_(connfd), connid_(connid){
    if (loop_ != nullptr) {
        channel_ = std::make_unique<Channel>(connfd, loop_);
        channel_ -> EnableET();
        channel_ -> set_read_callback(std::bind(&TcpConnection::HandleMessage, this));
        channel_ -> set_write_callback(std::bind(&TcpConnection::HandleWrite,this));
        //channel_ -> EnableRead();
    }
    read_buf_ = std::make_unique<Buffer>();
    send_buf_ = std::make_unique<Buffer>();

    context_ = std::make_unique<HttpContext>();
}

TcpConnection::~TcpConnection(){
    ::close(connfd_);
}

void TcpConnection::ConnectionEstablished(){
    state_ = ConnectionState::Connected;
    channel_ -> Tie(shared_from_this());
    channel_ -> EnableRead();
    if (on_connect_) {
        on_connect_(shared_from_this());
    }
}

void TcpConnection::ConnectionDestructor(){
    loop_ -> DeleteChannel(channel_.get());
}

void TcpConnection::set_connection_callback(std::function<void (const std::shared_ptr<TcpConnection> &)> const& fn) {
    on_connect_ = std::move(fn);
}

void TcpConnection::set_close_callback(std::function<void (const std::shared_ptr<TcpConnection> &)> const& fn) {
    on_close_ = std::move(fn);
}

void TcpConnection::set_message_callback(std::function<void(const std::shared_ptr<TcpConnection> &)> const &fn) { 
    on_message_ = std::move(fn);
}

void TcpConnection::HandleMessage(){
    Read();
    if (on_message_) {
        on_message_(shared_from_this());
    }
}

void TcpConnection::HandleWrite(){
    LOG_INFO << "TcpConnection::HandleWrite";
    WriteNonBlocking();
}

void TcpConnection::HandleClose(){
    if (state_ != ConnectionState::Disconnected) {
        state_ = ConnectionState::Disconnected;
        if (on_close_) {
            on_close_(shared_from_this());
        }
    }
}

EventLoop *TcpConnection::loop() const { return loop_; }
int TcpConnection::id() const { return connid_; }
int TcpConnection::fd() const { return connfd_; }
TcpConnection::ConnectionState TcpConnection::state() const { return state_; }
//void TcpConnection::set_send_buf(const char *str) { send_buf_->set_buf(str); }
Buffer *TcpConnection::read_buf(){ return read_buf_.get(); }
Buffer *TcpConnection::send_buf() { return send_buf_.get(); }

void TcpConnection::Read(){
    read_buf_ -> ReSetIndex();
    ReadNonBlocking();
}

void TcpConnection::Send(const char* msg){
    Send(msg, static_cast<int>(strlen(msg)));
}
void TcpConnection::Send(const std::string &msg){
    Send(msg.data(), static_cast<int>(msg.size()));
}
void TcpConnection::Send(const char *msg, int len){
    int remaining = len;
    int send_size = 0;
    if (send_buf_ -> readablebytes() == 0) {
        send_size = static_cast<int>(write(connfd_, msg, len));
        if (send_size >= 0) {
            remaining -= send_size;
        } else if ((send_size == -1) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            //说明此时TCP缓冲区是慢的，没有办法写入，什么都不做
            send_size = 0;// 说明实际上没有发送数据
        } else {
            LOG_ERROR << "TcpConnection::Send - TcpConnection Send ERROR";
            HandleClose();
            return;
        }
    }
    assert(remaining <= len);
    if (remaining > 0) {
        send_buf_ -> Append(msg + send_size, remaining);

        channel_ -> EnableWrite();
    }
}

void TcpConnection::Write(){
    WriteNonBlocking();
    send_buf_ -> ReSetIndex();
}

void TcpConnection::ReadNonBlocking(){
    char buf[READ_BUFFER];
    while (true) {
        memset(buf, 0, sizeof(buf));
        printf("start read\n");
        ssize_t bytes_read = read(connfd_, buf, sizeof(buf));
        if (bytes_read > 0) {
            read_buf_-> Append(buf, bytes_read);
        } else if (bytes_read == -1 && errno == EINTR) {  // 程序正常中断、继续读取
            printf("continue reading\n");
            continue;
        } else if (bytes_read == -1 &&
               ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞IO，这个条件表示数据全部读取完毕
            break;
        } else if (bytes_read == 0) {  // EOF，客户端断开连接
            printf("read EOF, client fd %d disconnected\n", connfd_);
            //loop_ -> DeleteChannel(channel_.get());
            HandleClose();
            break;
        } else {
            printf("Other error on client fd %d\n", connfd_);
            HandleClose();
            break;
        }
    }
}

void TcpConnection::WriteNonBlocking() {
    int remaining = send_buf_->readablebytes();
    int send_size = static_cast<int>(write(connfd_, send_buf_->Peek(), remaining));
    if((send_size == -1) && 
                ((errno == EAGAIN) || (errno == EWOULDBLOCK))){
        // 说明此时TCP缓冲区是满的，没有办法写入，什么都不做 
        // 主要是防止，在Send时write后监听EPOLLOUT，但是TCP缓冲区还是满的，
        send_size = 0;
    }
    else if (send_size == -1){
        LOG_ERROR << "TcpConnection::Send - TcpConnection Send ERROR";
    }

    remaining -= send_size;
    send_buf_->UpdataIndex(send_size);
}

HttpContext* TcpConnection::context() const{
    return context_.get();
}

TimeStamp TcpConnection::timestamp() const{ return timestamp_;}

void TcpConnection::UpdataTimeStamp(TimeStamp now){
    timestamp_ = now;
}