#ifndef _QUEUE_H_
#define _QUEUE_H_

#define MAX_TASKS 0x00FFFFFF

#include <stdbool.h>

typedef struct queue_t queue_t;

queue_t* queue_create();

bool queue_push(queue_t *q, void *ele);

void* queue_pop(queue_t *q);

void queue_destroy(queue_t *q);

bool queue_full(queue_t *q);

bool queue_empty(queue_t *q);
#endif
