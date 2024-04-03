#include "Server.h"
#include "Event.h"
//#include "ThreadPool.hpp"

std::vector<std::unique_ptr<Event>> g_events;

void Server::toUpperAndSend(int fd, const std::string& str, Event* ev) {
    std::string upperStr = str;
    std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    send(fd, upperStr.c_str(), upperStr.size(), 0);//发送回客户端
}

void Server::recvdata(int fd, int events, void* arg) {
    Event* ev = static_cast<Event*>(arg);
    char buf[MAXLINE] = {0};
    int len = recv(fd, buf, sizeof(buf), 0);//读文件描述符 数据存入buf中
    if (len > 0) {
        // 将接收到的字符串和对应的事件对象传递给线程池执行任务
        threadPool.add_task([=]() {
            toUpperAndSend(fd, buf, ev);
        });
    } else if (len == 0) {
        close(ev->fd);
        std::cout << "客户端" << ev->fd << "已关闭" << std::endl;
        ev->eventdel(g_efd, ev);
    } else {
        close(ev->fd);
        std::cout << "接收错误" << std::endl;
        ev->eventdel(g_efd, ev);
    }
}

void Server::senddata(int fd, int events, void* arg) {
    Event* ev = static_cast<Event*>(arg);

    threadPool.add_task([&]() {
        int len = send(fd, ev->buf.c_str(), ev->buf.size(), 0);
        if (len <= 0) {
            close(ev->fd);
            std::cout << "send error" << std::endl;
            ev->eventdel(g_efd, ev);
        } else {
            ev->eventdel(g_efd, ev);
        }
    });
}

void Server::acceptcon(int lfd, int events, void* arg) {
    struct sockaddr_in clit_addr;
    socklen_t clit_addr_len = sizeof(clit_addr);
    int cfd = accept(lfd, (struct sockaddr*)&clit_addr, &clit_addr_len);
    if (cfd == -1)
        sys_err("accept error");
    char str[INET_ADDRSTRLEN];
    //输出客户端的网络地址
    std::cout << "client_IP = " <<
        inet_ntop(AF_INET, &clit_addr.sin_addr.s_addr, str, 1024)
        << " client_Port = " << ntohs(clit_addr.sin_port) << std::endl;
    if (g_events.size() >= MAX_EVENTS) {
        close(cfd);
        std::cout << "too many clients" << std::endl;
        return;
    }

    int flag = fcntl(cfd, F_SETFL, O_NONBLOCK);
    if (flag < 0)
        sys_err("fcntl nonblocking");

    auto ev = std::make_unique<Event>();
    std::function<void(int, int, void*)> recvDataFunc = [&](int fd, int events, void* arg) {
        recvdata(fd, events, arg);
    };
    ev->eventset(ev.get(), cfd, recvDataFunc, ev.get());
    ev->eventadd(g_efd, EPOLLIN | EPOLLET, ev.get());
    g_events.push_back(std::move(ev));
}

void Server::init_listen_socket(int efd){
    std::cout << "init_listen_socket" <<std::endl;
    struct sockaddr_in serv_addr;
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(lfd, F_SETFL, O_NONBLOCK);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        sys_err("bind error");
    if (listen(lfd, MAX_LISTEN) == -1)
        sys_err("listen error");

    auto ev = std::make_unique<Event>();
    std::function<void(int, int, void *)> AcceptFunc = [&](int fd, int events, void *arg)
    { acceptcon(fd, events, arg); };
    ev->eventset(ev.get(), lfd, AcceptFunc, ev.get());
    ev->eventadd(efd, EPOLLIN, ev.get());
    g_events.push_back(std::move(ev));
}

Server::Server(int g_efd):g_efd(g_efd), threadPool(5, 10, 100) {
    init_listen_socket(g_efd);
}

Server::~Server(){}

void Server::start(){
    struct epoll_event events[MAX_EVENTS + 1];//保存已经满足就绪事件的文件描述符
    int check_pos = 0, i;
    for (;;)
    {
        long now = time(nullptr); //当前时间

        for (i = 0; i < g_events.size(); ++i)
        {
            if (!g_events[i] ||g_events[i]->status != 1) // 不在红黑树上
                continue;
            long take_time = now - g_events[i]->last_active;
            if (take_time >= 60) {
                close(g_events[i]->fd);
                std::cout << g_events[i]->fd << " time out" << std::endl;
                g_events[i]->eventdel(g_efd, g_events[i].get()); //将该客户端从红黑树移除
            }
        }
        int nfd = epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);
        if (nfd < 0)
            break;
        for (int i = 0; i < nfd; ++i) {
            Event* ev = static_cast<Event*>(events[i].data.ptr);
            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) //读操作就绪
                ev->call_back(ev->fd, events[i].events, ev->arg);
            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) //写操作就绪
                ev->call_back(ev->fd, events[i].events, ev->arg);
        }

    }
}
