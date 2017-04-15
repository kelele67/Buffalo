#ifndef BF_REQUEST_H
#define BF_REQUEST_H

#include <time.h>
#include "bf_http.h"

#define BF_AGAIN EAGAIN

#define BF_HTTP_PARSE_INVALID_METHOD  10
#define BF_HTTP_PARSE_INVALID_REQUEST  11
#define BF_HTTP_PARSE_INVALID_HEADER  12

#define BF_HTTP_UNKNOWN  0x0001
#define BF_HTTP_GET  0x0002
#define BF_HTTP_HEAD  0x0004
#define BF_HTTP_POST  0x0008

#define BF_HTTP_OK  200

#define BF_HTTP_NOT_MODIFIED  304

#define BF_HTTP_NOT_FOUND  404

#define MAX_BUF 8124

typedef struct bf_request_s {
    void *root;
    int fd;
    int epfd;
    char buf[MAX_BUF];
    size_t pos, last;
    int state;
    void *request_start;
    void *method_end;
    int method;
    void *uri_start;
    void *uri_end;
    void *path_start;
    void *path_end;
    void *query_start;
    void *query_end;
    int http_major;
    int http_minor;
    void *request_end;

    struct bf_queue_t list; //储存http 的 header
    void *cur_header_key_start;
    void *cur_header_key_end;
    void *cur_header_value_start;
    void *cur_header_value_end;

    void *timer;
} bf_request_t;

typedef struct {
    int fd;
    int keep_alive;
    time_t mtime; //文件的修改时间
    int modified; /* compare If-modified-since field with mtime to decide whether the file is modified since last time*/
    int status;
} bf_out_t;

typedef struct bf_header_s {
    void *key_start, *key_end;
    void *value_start, *value_end;
    bf_queue_t list;
} bf_header_t;

typedef int (*bf_header_handler_pt)(bf_request_t *r, bf_out_t *o, char *data, int len);

typedef struct {
    char *name;
    bf_header_handler_pt handler;
} bf_header_handle_t;

void bf_handle_header(bf_request_t *r, bf_out_t *o);
int bf_close_conn(bf_request_t *r);

int bf_init_request_t(bf_request_t *r, int fd, int epfd, bf_conf_t *cf);
int bf_free_request_t(bf_request_t *r);

int bf_init_out_t(bf_out_t *o, int fd);
int bf_free_out_t(bf_out_t *o);

const char *get_shortmsg_from_status_code(int status_code);

extern bf_header_handle_t    bf_headers_in[];

#endif
