#include <stdio.h>
#include <stdlib.h>
#include "task_queue.h"



int init_queue(TaskQueue *q) {
    q->front = NULL;
    q->rear = NULL;
    q->done = false;

    if (pthread_mutex_init(&q->mutex, NULL) != 0) {
        return 1;
    }

    if (pthread_cond_init(&q->not_empty, NULL) != 0) {
        pthread_mutex_destroy(&q->mutex);
        return 1;
    }

    return 0;
}

int is_empty(TaskQueue *q) {
    // si ambos estan nulos entonces está vacía
    return q->front == NULL;
}

int enqueue(TaskQueue *q, const Task *task) {

    Node *temp = malloc(sizeof(Node));
    if (temp == NULL) {
        printf("Error reservando memoria\n");
        return 1;
    }

    temp->value = *task;
    temp->next = NULL;

    pthread_mutex_lock(&q->mutex);
    if (is_empty(q)) {
        q->front = temp;
        q->rear = temp;
    } else {
        q->rear->next = temp;
        q->rear = temp;
    }

    // Con esto se avisa a los hilos que hay un nuevo task en la cola
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);

    return 0;
}

int dequeue_wait(TaskQueue *q, Task *out_task) {
    pthread_mutex_lock(&q->mutex);

    while (is_empty(q) && !q->done) {
        // se queda esperando y deja pasar a otro hilo
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    // Ahora bien, si está vacía y ya no hay tareas entonces se deja de esperar
    if (is_empty(q) && q->done) {
        pthread_mutex_unlock(&q->mutex);
        return 1; // retorna 1 al no haber tareas
    }

    Node *temp = q->front;
    *out_task = temp->value;

    q->front = q->front->next;
    if (is_empty(q)) {
        q->rear = NULL;
    }

    pthread_mutex_unlock(&q->mutex);
    free(temp);

    return 0;
}

void finish_queue(TaskQueue *q){
    pthread_mutex_lock(&q->mutex);
    q->done = true;
    // despierta todos los que estaban haciendo pthread_cond_wait
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

void queue_destroy(TaskQueue *q) {
    Node *curr = q->front;
    while (curr != NULL) {
        Node *tmp = curr;
        curr = curr->next;
        free(tmp);
    }

    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_empty);
}


// Esto es solo para debuggear que sirva
void print_queue(TaskQueue *q){
    Node *curr;
    curr = q->front;

    for(int i = 0; curr != NULL; i++){
        printf("Nodo %d:%s\n", i, curr->value.source_path);
        curr = curr->next;
    }

}
