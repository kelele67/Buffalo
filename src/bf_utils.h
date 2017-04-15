#ifndef BF_UTILS_H
#define BF_UTILS_H

#define LISTENQ 1024
#define BUFLEN 8192
#define DELIM "="
#define BF_CONF_OK 0
#define BF_CONF_ERROR 100

#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct bf_conf_s {
    void *root;
    int port;
    int thread_num;
};

typedef struct bf_conf_s bf_conf_t;

int open_listenfd(int port);
int make_socket_non_blocking(int fd);

int read_conf(char *filename, bf_conf_t *cf, char *buf, int len);

#endif