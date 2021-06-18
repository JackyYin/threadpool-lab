#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
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
    pthread_mutex_t   lock;
    pthread_cond_t    task_existence;
};

int threadpool_free(threadpool_t *pool);

void* threadpool_function (void *argument)
{
    threadpool_t *pool = (threadpool_t *)argument;
    threadpool_task_t *task = NULL;

    while (1) {
        pthread_mutex_lock(&pool->lock);

        while (queue_empty(pool->task_queue) && !pool->shutdown) {
            //printf("waiting for task_existence now...\n");
            pthread_cond_wait(&pool->task_existence, &pool->lock);
        }

        if (pool->shutdown)
            break;

        task = (threadpool_task_t *) queue_pop(pool->task_queue);
        pthread_mutex_unlock(&pool->lock);

        // queue is empty
        if (!task)
            continue;

        // receive final task
        if (!task->function) {
            free(task);
            task = NULL;
            break;
        }

        //printf("task received... %p %p %p \n", task, task->function, task->arguments);
        (*(task->function))(task->arguments);
        free(task);
        task = NULL;
    }

    pthread_mutex_unlock(&pool->lock);
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
        (pthread_mutex_init(&pool->lock, NULL) != 0) ||
        (pthread_cond_init(&pool->task_existence, NULL) != 0) ||
        (pool->task_queue == NULL) ||
        (pool->threads == NULL)
    ) {
        goto err;
    }

    for (int i = 0; i < thread_nums; i++) {
        if ((pthread_create(&pool->threads[i], NULL, threadpool_function, (void *)pool)) != 0) {
            goto err;
        }

        //printf("pthread num %d created...\n", i);
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
        // lock before we modify the index
        pthread_mutex_lock(&pool->lock);

        if (queue_full(pool->task_queue)) {
            code = 1;
            break;
        }

        threadpool_task_t *new_task = malloc(sizeof(threadpool_task_t));

        if (new_task) {
            new_task->function = func;
            new_task->arguments = args;

            while (!queue_push(pool->task_queue, (void *)new_task));

            if ((pthread_cond_signal(&pool->task_existence)) != 0) {
                printf("pthread_cond_signal failed... \n");
                free(new_task);
                code = 2;
                break;
            }
            //printf("task pushed... %p %p %p \n", new_task, new_task->function, new_task->arguments);
        } else {
            code = 3;
            break;
        }

    } while (0);

    pthread_mutex_unlock(&pool->lock);
    return code;
}

int threadpool_destroy (threadpool_t *pool)
{
    pool->shutdown = 1;

    if (pool) {
        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->task_existence);
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

    // notify all threads to begin
    pthread_cond_broadcast(&pool->task_existence);

    for (int i = 0; i < pool->thread_nums; i++) {
        printf("pthread_join res :%d \n", pthread_join(pool->threads[i], NULL));
    }

    threadpool_destroy(pool);
}
