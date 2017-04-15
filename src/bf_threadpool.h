#ifndef THREADPOOL_H
#define THREADPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include "bf_debug.h"

#define  THREAD_NUM 8

//封装线程池中的对象需要执行的任务对象
typedef struct bf_task_s {
    void (*func)(void *); //函数指针，需要执行的任务
    void *arg; // 参数
    struct bf_task_s *next; //任务队列中下一个任务
} bf_task_t;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t *threads;
    bf_task_t *head;
    int thread_count;
    int queue_size;
    int started;
    int shutdown;
} bf_threadpool_t;

typedef enum {
    bf_tp_invalid = -1,
    bf_tp_lock_fail = -2,
    bf_tp_already_shutdown = -3,
    bf_tp_cond_broadcast = -4,
    bf_tp_thread_fail = -5,

} bf_threadpool_error_t;

bf_threadpool_t *threadpool_init(int thread_num);

int threadpool_add(bf_threadpool_t *pool, void (*func)(void *), void *arg);

int threadpool_destroy(bf_threadpool_t *pool, int graceful);

#ifdef __cplusplus
}
#endif

#endif