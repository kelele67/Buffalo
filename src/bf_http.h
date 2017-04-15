#ifndef BF_HTTP_H
#define BF_HTTP_H

/*nginx在判断http method的时候用的不是字符串比较，而是整数比较。

比如“POST”，一般的写法是用strcmp，就会牵扯到4次char的比较。
而nginx把接受到的method转化为一个int，那么4次比较就可以转化为1次比较。*/


#include <string.h>
#include <stdint.h>
#include "bf_connection.h"
#include "bf_list.h"
#include "bf_debug.h"
#include "bf_utils.h"
#include "bf_request.h"

#define MAXLINE 8192
#define SHORTLINE 512

#define bf_str3_cmp(m, c0, c1, c2, c3) \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)
#define bf_str30cmp(m, c0, c1, c2, c3) \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)
#define bf_str4cmp(m, c0, c1, c2, c3)  \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

typedef struct mime_type_s {
    const char *type;
    const char *value;
} mime_type_t;

void do_request(void *infd);

#endif