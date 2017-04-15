#ifndef BF_TIMER_H
#define BF_TIMER_H

#include "bf_priority_queue.h"
#include "bf_request.h"

#define BF_TIMER_INFINITE -1
#define TIMEOUT_DEFAULT 500 // ms

typedef int (*timer_handler_pt)(bf_request_t *rq);

typedef struct bf_timer_node_s {
    size_t key;
    int deleted; //如果先关闭了socket客户端，将deleted设为1
    timer_handler_pt handler;
    bf_request_t *rq;
} bf_timer_node;

int bf_timer_init();
int bf_find_timer();
void bf_handle_expire_timers();

extern bf_pq_t bf_timer;
extern size_t bf_current_msec;

void bf_add_timer(bf_request_t *rq, size_t timeout, timer_handler_pt handler);
void bf_del_timer(bf_request_t *rq);

#endif