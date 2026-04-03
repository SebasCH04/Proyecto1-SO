#ifndef QUEUE_H
#define QUEUE_H

#define MAX_SIZE 1024

typedef struct node {
    char *data;
    struct node *next;
} Node;

typedef struct queue {
    int count;
    Node *front;
    Node *rear;
} Queue;

void init(Queue *q);
int isempty(Queue *q);
int full(Queue *q);
int enqueue(Queue *q, const char *value);
char *dequeue(Queue *q);
void destroy_queue(Queue *q);

#endif
