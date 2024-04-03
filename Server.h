#pragma once
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "func.h"
#include "ThreadPool.hpp"
#include "Event.h"

constexpr uint32_t SERV_PORT = 9527;
constexpr uint32_t MAX_LISTEN = 128;

class Server{
public:
    int g_efd;//保存epoll_create返回的文件描述符
    std::mutex g_mutex;
    ThreadPool threadPool; // 线程池
    Event evt;
    Server(int efd);
    ~Server();

    void toUpperAndSend(int fd, const std::string &str, Event *ev);//将字符串转换为大写形式
    void recvdata(int fd, int events, void *arg);
    void senddata(int fd, int events, void *arg);
    void acceptcon(int lfd, int events, void *arg);

    void init_listen_socket(int g_efd);
    void start();

};
