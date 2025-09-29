#pragma once
#include <functional>
#include <memory>
#include <stdio.h>
#include "commom.h"
#include "timestamp.h"

class TcpServer;
class TcpConnection;
class HttpRequest;
class HttpResponse;
class EventLoop;

#define AUTOCLOSETIMEOUT 5
class HttpServer{
public:
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void(const HttpRequest &, HttpResponse*)> HttpResponseCallback;

    DISALLOW_COPY_AND_MOVE(HttpServer);
    HttpServer(EventLoop* loop, const int port, bool auro_close_conn);
    ~HttpServer();

    void HttpDefaultCallBack(const HttpRequest &request, HttpResponse *resp);

    void SetHttpCallback(const HttpResponseCallback &cb);

    void start();

    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn);
    void onRequest(const TcpConnectionPtr &conn, const HttpRequest &request);

    void ActiveCloseConn(std::weak_ptr<TcpConnection> &conn);
    
    void SetThreadNums(int thread_nums);

    void TestTimer_IntervalEvery3Seconds() const {
        printf("%s TestTimer_IntervalEvery3Seconds\n", TimeStamp::Now().ToForMattedString().data());
        fflush(stdout);
    }
private:
    EventLoop *loop_;
    std::unique_ptr<TcpServer> server_;

    bool auto_close_conn_;
    HttpResponseCallback response_callback_;
};