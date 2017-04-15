#include "bf_threadpool.h"

typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown = 2
} bf_threadpool_sd_t;

static int threadpool_free(bf_threadpool_t *pool);
static void *threadpool_worker(void *arg);

// 初始化线程池
bf_threadpool_t *threadpool_init(int thread_num) {
    if (thread_num < 0) {
        log_err("the arg of threadpool_init must greater than 0");
        return NULL;
    }

    bf_threadpool_t *pool;
    if ((pool = (bf_threadpool_t *)malloc(sizeof(bf_threadpool_t))) == NULL) {
        goto err;
    }

    pool->thread_count = 0;
    pool->queue_size = 0;
    pool->shutdown = 0;
    pool->started = 0;
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);
    // 定义一个假头
    pool->head = (bf_task_t *)malloc(sizeof(bf_task_t));

    if ((pool->threads == NULL) || (pool->head == NULL)) {
        goto  err;
    }

    pool->head->func = NULL;
    pool->head->arg = NULL;
    pool->head->next = NULL;

    if (pthread_mutex_init(&(pool->lock), NULL) != 0) {
        goto err;
    }

    if (pthread_cond_init(&(pool->cond), NULL) != 0) {
        goto  err;
    }

    int i;
    for (i = 0; i < thread_num; ++i) {
        if (pthread_create(&(pool->threads[i]), NULL, threadpool_worker, (void*)pool) != 0) {
            threadpool_destroy(pool, 0);
            return NULL;
        }
        log_info("thread: %08x started", (uint32_t) pool->threads[i]);

        pool->thread_count++;
        pool->started++;
    }

    return pool;

err:
    if (pool) {
        threadpool_free(pool);
    }

    return NULL;
}

int threadpool_add(bf_threadpool_t *pool, void (*func)(void*), void *arg) {
    int ret, err = 0;
    if (pool == NULL || func == NULL) {
        log_err("pool == NULL or func == NULL");
        return -1;
    }

    if (pthread_mutex_lock(&(pool->lock)) != 0) {
        log_err("pehread_mutex_lock");
        return -1;
    }

    if (pool->shutdown) {
        err = bf_tp_already_shutdown;
        goto out;
    }

    //使用一个内存池
    bf_task_t *task = (bf_task_t *)malloc(sizeof(bf_task_t));
    if (task == NULL) {
        log_err("malloc task fail");
        goto out;
    }

    task->func = func;
    task->arg = arg;
    task->next = pool->head->next;
    pool->head->next = task;

    pool->queue_size++;

    ret = pthread_cond_signal(&(pool->cond));
    check(ret == 0, "pthread_cond_signal");

out:
    if (pthread_mutex_unlock(&pool->lock) != 0) {
        log_err("pthread_mutex_unlock");
        return -1;
    }
    return err;
}

int threadpool_free(bf_threadpool_t *pool) {
    if (pool == NULL || pool->started > 0) {
        return -1;
    }

    if (pool->threads) {
        free(pool->threads);
    }

    bf_task_t *old;
    /*pool->head 是我们之前定义的假头*/
    while (pool->head->next) {
        old = pool->head->next;
        pool->head->next = pool->head->next->next;
        free(old);
    }

    return 0;
}

// “优雅的” destroy 线程池
int threadpool_destroy(bf_threadpool_t *pool, int graceful) {
    int err = 0;

    if (pool == NULL) {
        log_err("pool == NULL");
        return bf_tp_invalid;
    }

    if (pthread_mutex_lock(&(pool->lock)) != 0) {
        return bf_tp_lock_fail;
    }

    do {/*建立线程睡眠标志 唤醒所有线程if*/
        if (pool->shutdown) {
            err = bf_tp_already_shutdown;
            break;
        }
        pool->shutdown = (graceful)? graceful_shutdown : immediate_shutdown;

        // 如果还有等待的线程
        if (pthread_cond_broadcast(&(pool->cond)) != 0) {
            err = bf_tp_cond_broadcast;
            break;
        }

        if (pthread_mutex_unlock(&(pool->lock)) != 0) {
            err = bf_tp_lock_fail;
            break;
        }

        int i;
        for (i = 0; i < pool->thread_count; ++i) {
            if (pthread_join(pool->threads[i], NULL) != 0) {
                err = bf_tp_thread_fail;
            }
            log_info("thread %08x exit", (uint32_t) pool->threads[i]);
        }
    } while(0);
    if (!err) {
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->cond));
        threadpool_free(pool);
    }

    return err;
}

static void *threadpool_worker(void *arg) {
    if (arg == NULL) {
        log_err("arg should be type bf_threadpool_t*");
        return NULL;
    }

    bf_threadpool_t *pool = (bf_threadpool_t *)arg;
    bf_task_t *task;

    while(1) {
        pthread_mutex_lock(&(pool->lock));

        //等待信号，检查假的唤醒
        while ((pool->queue_size == 0) && !(pool->shutdown)) {
            pthread_cond_wait(&(pool->cond), &(pool->lock));
        }

        if (pool->shutdown == immediate_shutdown) {
            break;
        }else if ((pool->shutdown == graceful_shutdown) && pool->queue_size == 0) {
            break;
        }

        task = pool->head->next;
        if (task == NULL) {
            pthread_mutex_unlock(&(pool->lock));
            continue;
        }

        pool->head->next = task->next;
        pool->queue_size--;

        pthread_mutex_unlock(&(pool->lock));

        (*(task->func))(task->arg);
        // 释放内存
        free(task);
    }

    pool->started--;
    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);

    return NULL;
}