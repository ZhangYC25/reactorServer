#include "../base/commom.h"
#include "eventloopthread.h"

#include <vector>
#include <memory>

class EventLoopThreadPool{
public:
    DISALLOW_COPY_AND_MOVE(EventLoopThreadPool);
    EventLoopThreadPool(EventLoop*);
    ~EventLoopThreadPool();

    void SetThreadNums(int thread_nums);
    void Start();
    EventLoop* nextloop();
private:
    EventLoop* main_loop_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    int thread_nums_;
    std::vector<EventLoop*> loops_;
    int next_;
};