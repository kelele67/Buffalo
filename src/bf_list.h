#ifndef BF_LIST_H
#define BF_LIST_H

#ifndef NULL
#define NULL 0
#endif

/*用来存储header*/
/*在 buffalo 的队列链表中，其维护的是指向链表节点的指针，
 * 并没有实际的数据区，所有对实际数据的操作需要我们自行操作，
 * 队列链表实质是双向循环链表，其操作是双向链表的基本操作*/
/*参考Nginx的实现也就是Linux内核中链表的实现方式*/
/*将链表塞入数据结构！*/

struct bf_queue_t {
    struct bf_queue_t *next;
    struct bf_queue_t *prev;
};

typedef struct bf_queue_t bf_queue_t;

#define INIT_BF_QUEUE_T(ptr) do {\
    struct bf_queue_t *_ptr = (struct bf_queue_t *) ptr;\
    (_ptr)->next = (_ptr); (_ptr)->prev = (_ptr);\
} while(0)

//插入新的节点
static inline void __list_add(struct bf_queue_t *_new, struct bf_queue_t *prev, struct bf_queue_t *next) {
    _new->next = next;
    next->prev = _new;
    prev->next = _new;
    _new->prev = prev;
}

static inline void list_add(struct bf_queue_t *_new, struct bf_queue_t *head) {
    __list_add(_new, head, head->next);
}

static inline void list_add_tail(struct bf_queue_t *_new, struct bf_queue_t *head) {
    __list_add(_new, head->prev, head);
}

//删除节点
static inline void __list_del(struct bf_queue_t *prev, struct bf_queue_t *next) {
    prev->next = next;
    next->prev = prev;
}

static inline void list_del(struct bf_queue_t *entry) {
    __list_del(entry->prev, entry->next);
}

static inline int list_empty(struct bf_queue_t *head) {
    return (head->next == head) && (head->prev == head);
}

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                  \
        const typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr -offsetof(type, member));  \
})

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

#endif
