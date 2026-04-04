#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include "task_queue.h"

typedef struct {
    pthread_t *threads;
    int num_threads;
    TaskQueue *queue;
} ThreadPool;

int  thread_pool_init(ThreadPool *pool, int num_threads, TaskQueue *queue);
void thread_pool_wait(ThreadPool *pool);
void thread_pool_destroy(ThreadPool *pool);

#endif
