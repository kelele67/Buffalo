//如果某个线程在处理fd的同时，又有新的一批数据发来，该fd可读（注意和上面那个问题的区别，一个是处理同一批数据时，一个是处理新来的一批数据），那么该fd会被分给另一个线程，这样两个线程处理同一个fd肯定就不对了。
//答：用EPOLLONESHOT解决这个问题。在fd返回可读后，需要显式地设置一下才能让epoll重新返回这个fd。

//我们需要用epoll的ET，用法的模式是固定的，把fd设为nonblocking，如果返回某fd可读，循环read直到EAGAIN（如果read返回0，则远端关闭了连接）

#include "bf_epoll.h"
#include "bf_debug.h"

struct epoll_event *events;

int bf_epoll_create(int size) {
    int efd = epoll_create1(size);
    check(efd > 0, "bf_epoll_create: epoll_create1");

    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAXEVENTS);
    check(efd > 0, "bf_epoll_create: malloc");
    return efd;
}

void bf_epoll_add(int epfd, int fd, struct  epoll_event *event) {
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, event);
    check(ret == 0, "bf_epoll_add: epoll_ctl");
    return;
}

void bf_epoll_mod(int epfd, int fd, struct epoll_event *event) {
    int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event);
    check(ret == 0, "bf_epoll_mod: epoll_ctl");
}

void bf_epoll_del(int epfd, int fd, struct epoll_event *event) {
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, event);
    check(ret == 0, "bf_epoll_del: epoll_ctl");
}

int bf_epoll_wait(int epfd, struct epoll_event *events, int max_events, int timeout) {
    int ret = epoll_wait(epfd, events, max_events, timeout);
    check(ret >= 0, "bf_epoll_wait: epoll_wait");
    return ret;
}
