#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "queue.h"

struct queue_t {
    int  head;
    int  tail;
    int  tasks_capacity;
    int  tasks_nums;
    void **buf;
};

queue_t* queue_create(int tasks_capacity)
{
    if (tasks_capacity < 1 || tasks_capacity > MAX_TASKS) {
        return NULL;
    }

    queue_t* q;
    if ((q = malloc(sizeof(queue_t))) == NULL) {
        return NULL;
    }

    q->tasks_capacity = tasks_capacity;
    q->tasks_nums = 0;
    q->head = 0;
    q->tail = 0;
    if ((q->buf = malloc(sizeof(void *) * tasks_capacity)) == NULL) {
        free(q);
        return NULL;
    }
    memset(q->buf, 0, sizeof(void*) * tasks_capacity);

    return q;
}

bool queue_push(queue_t* q, void *ele)
{
    int new_tail = (q->tail + 1) % q->tasks_capacity;

    if (new_tail == q->head)
        return false;

    q->buf[q->tail] = ele;
    q->tail = new_tail;
    q->tasks_nums++;
    return true;
}

void* queue_pop(queue_t* q)
{
    if (q->head == q->tail)
        return NULL;

    void *task = q->buf[q->head];
    q->head = (q->head + 1) % q->tasks_capacity;
    q->tasks_nums--;

    return task;
}

void queue_destroy(queue_t *q)
{
    if (q) {
        free(q->buf);
        free(q);
    }
}

bool queue_full(queue_t *q)
{
    return q->tasks_nums == q->tasks_capacity;
}

bool queue_empty(queue_t *q)
{
    return q->tasks_nums == 0;
}

