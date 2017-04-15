

#ifndef BF_PRIORITY_QUEUE_H
#define BF_PRIORITY_QUEUE_H

#include "bf_debug.h"
#include "bf_error.h"

#define BF_PQ_DEFAULT_SIZE 10

typedef int (*bf_pq_comparator_pt)(void *pi, void *pj);

typedef struct {
    void **pq;
    size_t nalloc;
    size_t size;
    bf_pq_comparator_pt comp;
} bf_pq_t;

int bf_pq_init(bf_pq_t *bf_pq, bf_pq_comparator_pt comp, size_t size);
int bf_pq_is_empty(bf_pq_t *bf_pq);
size_t bf_pq_size(bf_pq_t *bf_pq);
void *bf_pq_min(bf_pq_t *bf_pq);
int bf_pq_delmin(bf_pq_t *bf_pq);
int bf_pq_insert(bf_pq_t *bf_pq, void *item);

int bf_pq_sink(bf_pq_t *bf_pq, size_t i);

#endif