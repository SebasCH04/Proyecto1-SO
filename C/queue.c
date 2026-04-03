#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

typedef struct node {
    char *value;
    struct node *next;
} Node;

typedef struct queue {
    int count;
    Node *front;
    Node *rear;
} Queue;

void init(Queue *q) {
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}

int isempty(Queue *q) {
    return q->count == 0;
}

int full(Queue *q) {
    return q->count == MAX_SIZE;
}

int enqueue(Queue *q, const char *value) {
    if (full(q)) {
        printf("La cola está llena\n");
        return 0;
    }

    Node *temp = malloc(sizeof(Node));
    if (temp == NULL) {
        printf("Error reservando memoria\n");
        return 0;
    }

    temp->value = strdup(value);
    if (temp->value == NULL) {
        free(temp);
        printf("Error reservando memoria para el string\n");
        return 0;
    }

    temp->next = NULL;

    if (isempty(q)) {
        q->front = temp;
        q->rear = temp;
    } else {
        q->rear->next = temp;
        q->rear = temp;
    }

    q->count++;
    return 1;
}

char *dequeue(Queue *q) {
    if (isempty(q)) {
        printf("La cola ya está vacía\n");
        return NULL;
    }

    Node *p = q->front;
    char *x = p->value;

    q->front = q->front->next;
    q->count--;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    // liberar memoriua al final
    free(p);
    return x;
}

void destroy_queue(Queue *q) {
    while (!isempty(q)) {
        char *s = dequeue(q);
        free(s);
    }
}
