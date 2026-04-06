#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include "task_queue.h"

typedef struct {
    double exec_time;
    char file_name[PATH_MAX];
} CompletedTask;

typedef struct {
    TaskQueue *queue;
    char worker_id[16]; // Id para trackear luego estadísticas

    // Info de tareas
    CompletedTask *completed_tasks; // Una lista de tareas, no un puntero a una sola task
    int completed_count;
    int completed_capacity;
} WorkerInfo;

typedef struct {
    pthread_t *threads;
    WorkerInfo *workers; // esto es una lista de workers no un puntero a un solo worker
    int num_threads;
    TaskQueue *queue;
} ThreadPool;



int  thread_pool_init(ThreadPool *pool, int num_threads, TaskQueue *queue);
WorkerInfo * thread_pool_wait(ThreadPool *pool);
void thread_pool_destroy(ThreadPool *pool);
void free_completed_tasks(ThreadPool * pool, int num_threads);

#endif
