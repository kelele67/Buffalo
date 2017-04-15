#include "bf_priority_queue.h"
//用C写数据结构真是脑袋疼。。

int bf_pq_init(bf_pq_t *bf_pq, bf_pq_comparator_pt comp, size_t size) {
    bf_pq->pq = (void **)malloc(sizeof(void*) * (size + 1));
    if (!bf_pq->pq) {
        log_err("bf_pa_init: malloc failed");
        return -1;
    }

    bf_pq->nalloc = 0;
    bf_pq->size = size + 1;
    bf_pq->comp = comp;

    return BF_OK;
}

int bf_pq_is_empty(bf_pq_t *bf_pq) {
    return (bf_pq->nalloc == 0) ? 1: 0;
}

size_t bf_pq_size(bf_pq_t *bf_pq) {
    return bf_pq->nalloc;
}

void *bf_pq_min(bf_pq_t *bf_pq) {
    if (bf_pq_is_empty(bf_pq)) {
        return NULL;
    }
    return bf_pq->pq[1];
}

static int resize(bf_pq_t *bf_pq, size_t new_size) {
    if (new_size <= bf_pq->nalloc) {
        log_err("resize: new_size to samll");
        return -1;
    }

    void **new_ptr = (void**)malloc(sizeof(void*) * new_size);
    if (!new_ptr) {
        log_err("resize: malloc failed");
        return -1;
    }

    memcpy(new_ptr, bf_pq->pq, sizeof(void*) * (bf_pq->nalloc + 1));
    free(bf_pq->pq);
    bf_pq->pq = new_ptr;
    bf_pq->size = new_size;
    return BF_OK;
}

//交换
static void swap(bf_pq_t *bf_pq, size_t i, size_t j) {
    void *temp = bf_pq->pq[i];
    bf_pq->pq[i] = bf_pq->pq[j];
    bf_pq->pq[j] = temp;
}

static void swim(bf_pq_t *bf_pq, size_t k) {
    while (k > 1 && bf_pq->comp(bf_pq->pq[k], bf_pq->pq[k/2])) {
        swap(bf_pq, k, k/2);
        k /= 2;
    }
}

static size_t sink(bf_pq_t *bf_pq, size_t k) {
    size_t j;
    size_t nalloc = bf_pq->nalloc;

    while (2 * k <= nalloc) {
        j = 2 * k;
        if (j < nalloc && bf_pq->comp(bf_pq->pq[j+1], bf_pq->pq[j])) j++;
        if (!bf_pq->comp(bf_pq->pq[j], bf_pq->pq[k])) break;
        swap(bf_pq, j, k);
        k = j;
    }
    return k;
}

int bf_pq_delmin(bf_pq_t *bf_pq) {
    if (bf_pq_is_empty(bf_pq)) {
        return BF_OK;
    }
    swap(bf_pq, 1, bf_pq->nalloc);
    bf_pq->nalloc--;
    sink(bf_pq, 1);
    if (bf_pq->nalloc > 0 && bf_pq->nalloc <= (bf_pq->size - 1)/4) {
        if (resize(bf_pq, bf_pq->size / 2) < 0) {
            return -1;
        }
    }

    return BF_OK;
}

int bf_pq_insert(bf_pq_t *bf_pq, void *item) {
    //满了
    if (bf_pq->nalloc + 1 == bf_pq->size) {
        if (resize(bf_pq, bf_pq->size * 2) < 0) {
            return -1;
        }
    }

    bf_pq->pq[++bf_pq->nalloc] = item;
    swim(bf_pq, bf_pq->nalloc);

    return BF_OK;
}

int bf_pq_sink(bf_pq_t *bf_pq, size_t i) {
    return sink(bf_pq, i);
}