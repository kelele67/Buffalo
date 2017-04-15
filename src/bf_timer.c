#include <sys/time.h>
#include "bf_timer.h"

static int timer_comp(void *ti, void *tj) {
    bf_timer_node *timeri = (bf_timer_node *)ti;
    bf_timer_node *timerj = (bf_timer_node *)tj;

    return (timeri->key < timerj->key) ? 1 : 0;
}

bf_pq_t bf_timer;
size_t bf_current_msec;

static void bf_time_update() {
    //只有一个线程会同时访问，所以不需要加锁
    struct timeval tv;
    int ret;

    //获取时间
    ret = gettimeofday(&tv, NULL);
    check(ret == 0, "bf_time_update: gettimeofday error");

    bf_current_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    bf_debug("in bf_time_update, time = %zu", bf_current_msec);
}

int bf_timer_init() {
    int ret;
    ret = bf_pq_init(&bf_timer, timer_comp, BF_PQ_DEFAULT_SIZE);
    check(ret == BF_OK, "bf_pq_init error");

    bf_time_update();
    return BF_OK;
}

int bf_find_timer() {
    bf_timer_node *timer_node;
    int time = BF_TIMER_INFINITE;
    int ret;

    while (!bf_pq_is_empty(&bf_timer)) {
        bf_debug("bf_find_timer");
        bf_time_update();
        timer_node = (bf_timer_node *)bf_pq_min(&bf_timer);
        check(timer_node != NULL, "bf_pq_min error");

        if (timer_node->deleted) {
            ret = bf_pq_delmin(&bf_timer);
            check(ret == 0, "bf_pq_delmin");
            free(timer_node);
            continue;
        }

        time = (int) (timer_node->key - bf_current_msec);
        bf_debug("in bf_find_timer, key = %zu, cur = %zu", timer_node->key, bf_current_msec);
        time = (time > 0 ? time : 0);
        break;
    }

    return time;
}

//终止timer
//bf_handle_expire_timers
void bf_handle_expire_timers() {
    bf_debug("in bf_handle_expire_timers");
    bf_timer_node *timer_node;
    int ret;

    while (!bf_pq_is_empty(&bf_timer)) {
        bf_debug("bf_handle_expore_timers, size = %zu", bf_pq_size(&bf_timer));
        bf_time_update();
        timer_node = (bf_timer_node *)bf_pq_min(&bf_timer);
        check(timer_node != NULL, "bf_pq_min error");

        if (timer_node->deleted) {
            ret = bf_pq_delmin(&bf_timer);
            check(ret == 0, "bf_handle_expire_timers : bf_pq_delmin error");
            free(timer_node);
            continue;
        }

        if (timer_node->key > bf_current_msec) {
            return;
        }

        if (timer_node->handler) {
            timer_node->handler(timer_node->rq);
        }
        ret = bf_pq_delmin(&bf_timer);
        check(ret == 0, "bf_handle_expire_timers: bf_pq_delmin error");
        free(timer_node);
    }
}

void bf_add_timer(bf_request_t *rq, size_t timeout, timer_handler_pt handler) {
    int ret;
    bf_timer_node *timer_node = (bf_timer_node *)malloc(sizeof(bf_timer_node));
    check(timer_node != NULL, "bf_add_timer : malloc error");

    bf_time_update();
    rq->timer = timer_node;
    timer_node->key = bf_current_msec + timeout;
    bf_debug("in bf_add_timer, key = %zu", timer_node->key);
    timer_node->deleted = 0;
    timer_node->handler = handler;
    timer_node->rq = rq;

    ret = bf_pq_insert(&bf_timer, timer_node);
    check(ret == 0, "bf_add_timer: bf_pq_insert error");
}

void bf_del_timer(bf_request_t *rq) {
    bf_debug("in bf_del_timer");
    bf_time_update();
    bf_timer_node *timer_node = rq->timer;
    check(timer_node != NULL, "bf_del_timer : rq->timer is NULL");

    timer_node->deleted = 1;
}