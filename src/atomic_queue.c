#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
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

// we can support multiple producers
bool queue_push(queue_t* q, void *ele)
{
    do {
        uint32_t last_head = atomic_load(&q->head);
        uint32_t last_tail = atomic_load(&q->tail);
        uint32_t new_tail = (last_tail + 1) & (q->task_capacity - 1);

        if (new_tail == last_head)
            return false;

        // point to NULL before we move on,
        // becasue we don't want to cause seg fault after moving tail,
        // there should be no memory leakage becasue buffer should be freed
        q->buf[last_tail] = NULL;
        if (atomic_compare_exchange_weak(&q->tail, &last_tail, new_tail)) {
            // if buffer is immediately comsumed here,
            // it will get NULL pointer
            if (__sync_bool_compare_and_swap(&q->buf[last_tail], NULL, ele)) {
                return true;
            }
        }
    } while (1);
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

        if (last_head == last_tail) {
            //printf("queue_pop: queue is empty\n");
            return NULL;
        }

        // don't forward head if we get NULL pointer,
        // this means we are getting element before queue_push complete
        if ((res = q->buf[last_head]) == NULL) {
            printf("get NULL pointer when pop\n");
            continue;
        }
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
