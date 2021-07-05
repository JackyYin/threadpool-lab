#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include "threadpool.h"
#include "queue.h"

typedef struct {
    void (*function)(void*);
    void *arguments;
} threadpool_task_t;

struct threadpool_t {
    int               shutdown;
    int               thread_nums;
    queue_t           *task_queue;
    pthread_t         *threads;
};

int threadpool_free(threadpool_t *pool);

void* threadpool_function (void *argument)
{
    threadpool_t *pool = (threadpool_t *)argument;

    while (1) {
        if (pool->shutdown) {
            break;
        }

        int i = 0;
        threadpool_task_t *task;
        while ((task = (threadpool_task_t*) queue_pop(pool->task_queue)) == NULL) {
            printf("queue_pop iteration: %d\n", ++i);
            usleep(1000);
        }

        // receive final task
        if (!task->function) {
            free(task);
            break;
        }

        (*(task->function))(task->arguments);
        free(task);
    }

    pthread_exit(NULL);
}

threadpool_t* threadpool_create (int thread_nums, int tasks_capacity)
{
    if (thread_nums < 1 || thread_nums > MAX_THREADS) {
        return NULL;
    }

    threadpool_t* pool;
    if ((pool = malloc(sizeof(threadpool_t))) == NULL) {
        return NULL;
    }

    pool->thread_nums = thread_nums;
    pool->shutdown = 0;
    pool->threads = malloc(sizeof(pthread_t) * thread_nums);
    pool->task_queue = queue_create(tasks_capacity);

    if (
        (pool->task_queue == NULL) ||
        (pool->threads == NULL)
    ) {
        goto err;
    }

    for (int i = 0; i < thread_nums; i++) {
        if ((pthread_create(&pool->threads[i], NULL, threadpool_function, (void *)pool)) != 0) {
            goto err;
        }

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        if ((pthread_setaffinity_np(pool->threads[i], sizeof(cpuset), &cpuset)) != 0) {
            goto err;
        }

        printf("pthread number %d created in core %d...\n", i, i);
    }
    return pool;

err:
    threadpool_destroy(pool);
    return NULL;
}

int threadpool_push (threadpool_t *pool, void (*func)(void*), void *args)
{
    int code = 0;

    do {
        /* if (queue_full(pool->task_queue)) { */
        /*     code = 1; */
        /*     break; */
        /* } */

        threadpool_task_t *new_task = malloc(sizeof(threadpool_task_t));

        if (new_task) {
            new_task->function = func;
            new_task->arguments = args;

            int i = 0;
            while (!queue_push(pool->task_queue, (void *)new_task)) {
                printf("queue_push iteration: %d\n", ++i);
                usleep(10);
            }
        } else {
            code = 3;
            break;
        }

    } while (0);

    return code;
}

int threadpool_destroy (threadpool_t *pool)
{
    pool->shutdown = 1;

    if (pool) {
        queue_destroy(pool->task_queue);
        free(pool->threads);
        free(pool);
    }
    return 0;
}

void threadpool_wait (threadpool_t *pool)
{
    for (int i = 0; i < pool->thread_nums; i++) {
        threadpool_push(pool, NULL, NULL);
    }
    for (int i = 0; i < pool->thread_nums; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    threadpool_destroy(pool);
}
