#ifndef BF_CONNECTION_H
#define BF_CONNECTION_H

#include <sys/types.h>
#define CONN_BUFSIZE 8192

typedef struct {
    int conn_fd;
    ssize_t conn_cnt; //还没有读到的bytes
    char *conn_bufptr; //下一个要读的bytes
    char conn_buf[CONN_BUFSIZE];
} conn_t;

ssize_t conn_readn(int fd, void *usrbuf, size_t n);
ssize_t conn_writen(int fd, void *usrbuf, size_t n);
void conn_readinitb(conn_t *rp, int fd);
ssize_t conn_readnb(conn_t *rp, void *usrbuf, size_t maxlen);

#endif