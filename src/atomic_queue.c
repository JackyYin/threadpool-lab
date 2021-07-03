#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdint.h>
#include "queue.h"

struct queue_t {
    atomic_uint head;
    atomic_uint tail;
    int  task_capacity;
    void **buf;
};

queue_t* queue_create(int task_capacity)
{
    if (task_capacity < 1 || task_capacity > MAX_TASKS) {
        return NULL;
    }

    queue_t* q;
    if ((q = malloc(sizeof(queue_t))) == NULL) {
        return NULL;
    }

    q->task_capacity = task_capacity;
    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    if ((q->buf = malloc(sizeof(void *) * task_capacity)) == NULL) {
        free(q);
        return NULL;
    }

    return q;
}

// we can only allow single producer
bool queue_push(queue_t* q, void *ele)
{
    uint32_t last_head = atomic_load(&q->head);
    uint32_t last_tail = atomic_load(&q->tail);
    uint32_t new_tail = (last_tail + 1) & (q->task_capacity - 1);

    if (new_tail == last_head)
        return false;

    q->buf[last_tail] = ele;
    atomic_store_explicit(&q->tail, new_tail, memory_order_release);

    return true;
}

// we can have multiple consumers
void* queue_pop(queue_t* q)
{
    void * res;
    uint32_t last_head, last_tail, new_head;
    do {
        last_head = atomic_load(&q->head);
        last_tail = atomic_load(&q->tail);
        new_head = (last_head + 1) & (q->task_capacity - 1);

        if (last_head == last_tail)
            return NULL;

        res = q->buf[last_head];
    } while (!atomic_compare_exchange_weak(&q->head, &last_head, new_head));

    return res;
}

void queue_destroy(queue_t *q)
{
    if (q) {
        free(q->buf);
        free(q);
    }
}
