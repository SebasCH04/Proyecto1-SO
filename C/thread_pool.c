#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "thread_pool.h"

// copia el contenido de source_path hacia dest_path byte a byte usando read write
static int copy_file(const char *source_path, const char *dest_path) {
    int src_fd = open(source_path, O_RDONLY);
    if (src_fd == -1) {
        perror("Error abriendo archivo fuente");
        return 1;
    }

    // obtiene permisos del archivo original
    struct stat statbuf;
    if (fstat(src_fd, &statbuf) == -1) {
        perror("Error obteniendo permisos del archivo");
        close(src_fd);
        return 1;
    }

    int dst_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, statbuf.st_mode);
    if (dst_fd == -1) {
        perror("Error creando archivo destino");
        close(src_fd);
        return 1;
    }

    char buf[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buf, sizeof(buf))) > 0) {
        if (write(dst_fd, buf, bytes_read) != bytes_read) {
            perror("Error escribiendo archivo destino");
            close(src_fd);
            close(dst_fd);
            return 1;
        }
    }

    close(src_fd);
    close(dst_fd);
    return 0;
}

// funcion que ejecuta cada hilo del pool
// llama a dequeue_wait, si retorna 1 sale del loop, si retorna 0 copia el archivo al destino
// ojo que los directorios los crea el hilo principal en traverse_source
// esto es para que existan antes de que los workers copien sus archivos
static void *worker(void *arg) {
    TaskQueue *q = (TaskQueue *)arg;

    while (1) {
        Task task;
        int status = dequeue_wait(q, &task);

        // la cola esta terminada y vacia
        if (status != 0) {
            break;
        }

        if (copy_file(task.source_path, task.dest_path) == 0) {
            printf("[worker] archivo copiado: %s a %s\n",
                   task.source_path, task.dest_path);
        }
    }
    return NULL;
}

int thread_pool_init(ThreadPool *pool, int num_threads, TaskQueue *queue) {
    pool->num_threads = num_threads;
    pool->queue = queue;

    pool->threads = malloc(sizeof(pthread_t) * num_threads);
    if (pool->threads == NULL) {
        perror("Error reservando memoria para threads");
        return 1;
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker, queue) != 0) {
            perror("Error creando thread");
            // solo hace join a los que se crearon
            pool->num_threads = i; 
            return 1;
        }
    }
    return 0;
}

// espera a que todos los hilos terminen, se llama despues de finish_queue
void thread_pool_wait(ThreadPool *pool) {
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
}

void thread_pool_destroy(ThreadPool *pool) {
    free(pool->threads);
    pool->threads = NULL;
}
