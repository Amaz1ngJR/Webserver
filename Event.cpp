#include "Event.h"

void Event::eventset(Event* ev, int fd, std::function<void(int, int, void *)> call_back, void* arg) {
    ev->fd = fd;
    ev->events = 0;
    ev->buf.clear();
    ev->call_back = call_back;
    ev->arg = arg;
    ev->status = false;
    ev->last_active = time(nullptr);
}

void Event::eventadd(int efd, int events, Event* ev) {
    struct epoll_event epv = {0, {0}};
    int op;
    epv.data.ptr = ev;
    epv.events = ev->events = events;

    if (!ev->status) {
        op = EPOLL_CTL_ADD;
        ev->status = true;
    } 
    else 
        op = EPOLL_CTL_MOD;

    if (epoll_ctl(efd, op, ev->fd, &epv) < 0) 
        std::cout << "event " << ev->fd << " add " << events << " failed " << std::endl;
    else 
        std::cout << "event " << ev->fd << " add " << events << " success " << std::endl;
    
}

void Event::eventdel(int efd, Event* ev) {
    if (!ev->status)
        return;
    struct epoll_event epv = {0, {0}};
    epv.data.ptr = nullptr;
    ev->status = false;
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);
}
