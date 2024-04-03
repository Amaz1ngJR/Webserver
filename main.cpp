#include "ThreadPool.hpp"
#include "Event.h"
#include "Server.h"

int main() {
    process(2);
    //ThreadPool threadPool(5, 10, 100);//最小线程数、最大线程数、任务队列的最大长
    //Server server(epoll_create(MAX_EVENTS + 1));
    //server.start();
    // ThreadPool *thp = new ThreadPool(5, 10, 100);//创建线程池 线程数量为3-100 队列最大为100
    // printf("pool inited");
    Server server(epoll_create(MAX_EVENTS + 1));
    server.start();
    // int num[200];
    // for (int i = 0; i < 200;++i){
    //     num[i] = i;
    //     std::cout << "add task " << i << std::endl;
    //     thp->add_task(process,i);// 向线程池添加任务
    // }
    // while(!thp->task_queue.empty()){}
    // std::cout << "所有任务都分发完成" << std::endl;
    // sleep(10);//等待最后一个子线程完成任务
    return 0;
}
