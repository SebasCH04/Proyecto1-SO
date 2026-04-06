#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "thread_pool.h"

// copia el contenido de source_path hacia dest_path byte a byte usando read write y devuelve cuntos bytes copió
static ssize_t copy_file(const char *source_path, const char *dest_path) {
    int src_fd = open(source_path, O_RDONLY);
    if (src_fd == -1) {
        perror("Error abriendo archivo fuente");
        return -1;
    }

    // obtiene permisos del archivo original
    struct stat statbuf;
    if (fstat(src_fd, &statbuf) == -1) {
        perror("Error obteniendo permisos del archivo");
        close(src_fd);
        return -1;
    }

    int dst_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, statbuf.st_mode);
    if (dst_fd == -1) {
        perror("Error creando archivo destino");
        close(src_fd);
        return -1;
    }

    char buf[4096];
    ssize_t bytes_read;
    ssize_t total_bytes = 0;
    while ((bytes_read = read(src_fd, buf, sizeof(buf))) > 0) {
        if (write(dst_fd, buf, bytes_read) != bytes_read) {
            perror("Error escribiendo archivo destino");
            close(src_fd);
            close(dst_fd);
            return -1;
        }
        total_bytes += bytes_read;
    }

    close(src_fd);
    close(dst_fd);
    return total_bytes;
}

// funcion que ejecuta cada hilo del pool
// llama a dequeue_wait, si retorna 1 sale del loop, si retorna 0 copia el archivo al destino
// ojo que los directorios los crea el hilo principal en traverse_source
// esto es para que existan antes de que los workers copien sus archivos
static void *worker(void *arg) {
    WorkerInfo *info = (WorkerInfo *)arg;
    TaskQueue *q = info->queue;
    int *count = &info->completed_count;
    int *capacity = &info->completed_capacity;

    while (1) {
        Task task;
        int status = dequeue_wait(q, &task);

        // la cola esta terminada y vacia
        if (status != 0) {
            break;
        }

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        ssize_t copied_bytes = copy_file(task.source_path, task.dest_path);
        if (copied_bytes >= 0) {
            printf("archivo copiado: %s (%zd bytes)\n",
                   task.source_path,
                   copied_bytes);

            //revisar que aun haya campo en el arreglo
            if(*count == *capacity){

                CompletedTask *temp= realloc(info->completed_tasks, sizeof(CompletedTask) * (*capacity + 4));
                if (temp == NULL) {
                    return NULL;
                }
                *capacity += 4;
                info->completed_tasks = temp;
            }

            clock_gettime(CLOCK_MONOTONIC, &end);

            double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

            CompletedTask *new_task = &info->completed_tasks[*count];
            new_task->exec_time = time_taken;
            snprintf(new_task->file_name, PATH_MAX, "%s", task.source_path);
            (*count)++;
        }

    }

    return NULL;
}

int thread_pool_init(ThreadPool *pool, int num_threads, TaskQueue *queue) {
    pool->num_threads = num_threads;
    pool->queue = queue;

    pool->threads = malloc(sizeof(pthread_t) * num_threads);
    if (pool->threads == NULL) {
        perror("Error reservando memoria para threads\n");
        return 1;
    }

    pool->workers = malloc(sizeof(WorkerInfo) * num_threads);
    if(pool->workers == NULL){
        perror("Error reservando memoria para workers\n");
        free(pool->threads);
        return 1;
    }

    for (int i = 0; i < num_threads; i++) {

        pool->workers[i].queue = queue;
        snprintf(pool->workers[i].worker_id, sizeof(pool->workers[i].worker_id), "worker %d", i);

        //Inicializar los espacios de completed tasks
        pool->workers[i].completed_count = 0;
        pool->workers[i].completed_capacity = 4;
        pool->workers[i].completed_tasks = malloc(sizeof(CompletedTask) * pool->workers[i].completed_capacity);
        if(pool->workers[i].completed_tasks == NULL){
            printf("Error reservando memoria para completed task del worker %d\n",i);
            free(pool->threads);
            free_completed_tasks(pool, i); // a lo sumo se han creado i hilos
            free(pool->workers);
            return 1;
        }

        if (pthread_create(&pool->threads[i], NULL, worker, &pool->workers[i]) != 0) {
            printf("Error creando thread");
            // solo hace join a los que se crearon
            pool->num_threads = i;
            return 1;
        }
    }
    return 0;
}

// espera a que todos los hilos terminen, se llama despues de finish_queue
WorkerInfo *thread_pool_wait(ThreadPool *pool) {

    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    return pool->workers;
}

void thread_pool_destroy(ThreadPool *pool) {
    free(pool->threads);
    free_completed_tasks(pool, pool->num_threads);
    free(pool->workers);
    pool->threads = NULL;
    pool->workers = NULL;
}



// función para liberar el espacio de los completed tasks
// lo puse en una función por separado por orden
void free_completed_tasks(ThreadPool * pool, int num_threads){
    for(int i = 0; i < num_threads; i++){
        if(pool->workers[i].completed_tasks != NULL){
            free(pool->workers[i].completed_tasks);
            pool->workers[i].completed_tasks = NULL;
        }
    }
}
