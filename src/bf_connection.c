#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "bf_debug.h"
#include "bf_connection.h"

ssize_t conn_readn(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char*)usrbuf;

    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR) {
                nread = 0;
            } else {
                return -1;
            }
        }else if (nread == 0) {
            break;
        }
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);  /*return >=0 */
}

ssize_t conn_writen(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = (char *)usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno ==EINTR) {
                nwritten = 0;
            } else {
                log_err("errno = %d\n", errno);
                return -1;
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }

    return n;
}

static ssize_t conn_read(conn_t *rp, char *usrbuf, size_t n) {
    size_t cnt;

    //如果缓存区还不满
    while (rp->conn_cnt <= 0) {
        rp->conn_cnt = read(rp->conn_fd, rp->conn_buf, sizeof(rp->conn_buf));
        if (rp->conn_cnt < 0) {
            if (errno == EAGAIN) {
                //已经读完了所有的数据
                return -EAGAIN;
            }

            if (errno != EINTR) {
                //被信号打断
                return -1;
            }
        } else if (rp->conn_cnt == 0) {
            return 0;
        } else {
            rp->conn_bufptr = rp->conn_buf; //重置buffer指针
        }
    }

        /* Copy min(n, rp->rio_cnt) 的数据从网络buf到用户buf*/
        cnt = n;
        if (rp->conn_cnt < (ssize_t)n) {
            cnt = rp->conn_cnt;
        }
        memcpy(usrbuf, rp->conn_bufptr, cnt);
        rp->conn_bufptr += cnt;
        rp->conn_cnt -= cnt;
        return cnt;
}

void conn_readinitb(conn_t *rp, int fd) {
    rp->conn_fd = fd;
    rp->conn_cnt = 0;
    rp->conn_bufptr = rp->conn_buf;
}

ssize_t conn_readnb(conn_t *rp, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char*)usrbuf;

    while (nleft > 0) {
        if ((nread = conn_read(rp, bufp, nleft)) < 0) {
            if (errno == EINTR) {
                nread = 0;
            } else {
                return -1;
            }
        } else if (nread == 0) { // EOF
            break;
        }
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}

ssize_t conn_readlineb(conn_t *rp, void *usrbuf, size_t maxlen) {
    size_t n;
    ssize_t rc;
    char c, *bufp = (char *)usrbuf;

    for (n = 1; n < maxlen; n++) {
        if ((rc = conn_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n') {
                break;
            }
        } else if (rc == 0) {
            if (n == 1) {
                return 0; /* EOF, 没有可读的数据 */
            } else {
                break;  /* EOF, 数据已经被读过了 */
            }
        } else if (rc == -EAGAIN) {
            // 再读一次
            return rc;
        } else {
            return -1;
        }
    }
    *bufp = 0;
    return n;
}