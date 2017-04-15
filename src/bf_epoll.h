#ifndef BF_EPOLL_H
#define BF_EPOLL_H

#include <sys/epoll.h>

#define MAXEVENTS 1024

int bf_epoll_create(int size);
void bf_epoll_add(int epfd, int fs, struct epoll_event *event);
void bf_epoll_mod(int epfd, int fs, struct epoll_event *event);
void bf_epoll_del(int epfd, int fs, struct epoll_event *event);
int bf_epoll_wait(int epfd, struct epoll_event *events, int max_events, int timeout);

#endif

