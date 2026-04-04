#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <stdbool.h>
#include <pthread.h>
#include <limits.h>

typedef struct task {
    char source_path[PATH_MAX];
    char dest_path[PATH_MAX];
} Task;

typedef struct node {
    Task value;
    struct node *next;
} Node;

typedef struct TaskQueue {
    Node *front;
    Node *rear;
    bool done;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
} TaskQueue;

int init_queue(TaskQueue *q);
int is_empty(TaskQueue *q);
int enqueue(TaskQueue *q, const Task *task);
int dequeue_wait(TaskQueue *q, Task *out_task);
void finish_queue(TaskQueue *q);
void queue_destroy(TaskQueue *q);
void print_queue(TaskQueue *q);
#endif
