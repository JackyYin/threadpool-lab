#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#define MAX_THREADS 64

typedef struct threadpool_t threadpool_t;

threadpool_t* threadpool_create(int thread_nums, int task_capacity);

int threadpool_push(threadpool_t *pool, void (*func)(void *), void *arg);

int threadpool_destroy(threadpool_t *pool);

void threadpool_wait (threadpool_t *pool);
#endif
