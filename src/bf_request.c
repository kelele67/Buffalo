#ifndef _GNU_SOURCE
/* why define _GNU_SOURCE? http://stackoverflow.com/questions/15334558/compiler-gets-warnings-when-using-strptime-function-ci */
#define _GNU_SOURCE
#endif

#include <math.h>
#include <time.h>
#include <unistd.h>
#include "bf_http.h"
#include "bf_request.h"
#include "bf_error.h"

static int bf_process_ignore(bf_request_t *r, bf_out_t *out, char *data, int len);
static int bf_process_connection(bf_request_t *r, bf_out_t *out, char *data, int len);
static int bf_process_if_modified_since(bf_request_t *r, bf_out_t *out, char *data, int len);

bf_header_handle_t bf_headers_in[] = {
        {"Host", bf_process_ignore},
        {"Connection", bf_process_connection},
        {"If-Modified-Since", bf_process_if_modified_since},
        {"", bf_process_ignore}
};

int bf_init_request_t(bf_request_t *r, int fd, int epfd, bf_conf_t *cf) {
    r->fd = fd;
    r->epfd = epfd;
    r->pos = r->last = 0;
    r->state = 0;
    r->root = cf->root;
    INIT_BF_QUEUE_T(&(r->list));

    return BF_OK;
}

int bf_free_request_t(bf_request_t *r) {
    // TODO
    (void) r;
    return BF_OK;
}

int bf_init_out_t(bf_out_t *o, int fd) {
    o->fd = fd;
    o->keep_alive = 0;
    o->modified = 1;
    o->status = 0;

    return BF_OK;
}

int bf_free_out_t(bf_out_t *o) {
    // TODO
    (void) o;
    return BF_OK;
}

void bf_handle_header(bf_request_t *r, bf_out_t *o) {
    bf_queue_t *pos;
    bf_header_t *hd;
    bf_header_handle_t *header_in;
    int len;

    list_for_each(pos, &(r->list)) {
        hd = list_entry(pos, bf_header_t, list);
        // handle
        for (header_in = bf_headers_in;
             strlen(header_in->name) > 0;
             header_in++) {
            if (strncmp(hd->key_start, header_in->name, hd->key_end - hd->key_start) == 0) {
                len = hd->value_end - hd->value_start;
                (*(header_in->handler))(r, o, hd->value_start, len);
                break;
            }
        }
        list_del(pos);
        free(hd);
    }
}

int bf_close_conn(bf_request_t *r) {
    // NOTICE: closing a file descriptor will cause it to be removed from all epoll sets automatically
    // http://stackoverflow.com/questions/8707601/is-it-necessary-to-deregister-a-socket-from-epoll-before-closing-it
    close(r->fd);
    free(r);

    return BF_OK;
}

static int bf_process_ignore(bf_request_t *r, bf_out_t *out, char *data, int len) {
    (void) r;
    (void) out;
    (void) data;
    (void) len;

    return BF_OK;
}

static int bf_process_connection(bf_request_t *r, bf_out_t *out, char *data, int len) {
    (void) r;
    if (strncasecmp("keep_alive", data, len) == 0) {
        out->keep_alive = 1;
    }

    return BF_OK;
}

static int bf_process_if_modified_since(bf_request_t *r, bf_out_t *out, char *data, int len) {
    (void) r;
    (void) len;

    struct tm tm;
    if (strptime(data, "%a, %d %b %Y %H:%S GMT", &tm) == (char*)NULL) {
        return BF_OK;
    }
    time_t client_time = mktime(&tm);

    double time_diff = difftime(out->mtime, client_time);
    if (fabs(time_diff) < 1e-6) {
        //时间没有被修改
        out->modified = 0;
        out->status = BF_HTTP_NOT_MODIFIED;
    }

    return BF_OK;
}

const char *get_shortmsg_from_status_code(int status_code) {
    /*各个状态码:
    * http://users.polytech.unice.fr/~buffa/cours/internet/POLYS/servlets/Servlet-Tutorial-Response-Status-Line.html
    */
    if (status_code == BF_HTTP_OK) {
        return "OK";
    }

    if (status_code == BF_HTTP_NOT_MODIFIED) {
        return "Not Modified";
    }

    if (status_code == BF_HTTP_NOT_FOUND) {
        return "Not Found";
    }

    return "Unknown";
}