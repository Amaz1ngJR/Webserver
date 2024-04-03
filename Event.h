#pragma once
#include <sys/epoll.h>
#include "func.h"

constexpr uint32_t MAXLINE = 8192;
constexpr uint32_t MAX_EVENTS = 1024;

class Event{
public:
    int fd;  //要监听的文件描述符
    int events; //对应的监听事件
    void* arg; //泛型参数
    std::string buf;
    std::function<void(int, int, void*)> call_back;//回调函数
    long last_active;//记录加入红黑树的时间
    bool status;//是否监听

    void eventset(Event *ev, int fd, std::function<void(int, int, void *)> call_back, void *arg);
    void eventadd(int efd, int events, Event *ev);
    void eventdel(int efd, Event *ev);
};
