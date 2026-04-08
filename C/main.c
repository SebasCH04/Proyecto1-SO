// para pruebas de rendimiento
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "task_queue.h"
#include "fs_mgmt.h"
#include "thread_pool.h"
#include "logging.h"

// numero default de hilos 
#define DEFAULT_NUM_THREADS 4

int main(int argc, char *argv[]){

    if (argc < 3 || argc > 4){
        printf("Uso: cp source_dir destiny_dir [num_hilos]\n");
        return 1;
    }

    char *source_dir = argv[1];
    char *destiny_dir = argv[2];
    int num_threads = DEFAULT_NUM_THREADS;

    if (argc == 4) {
        num_threads = atoi(argv[3]);
        if (num_threads < 1) {
            printf("El numero de hilos debe ser al menos 1\n");
            return 1;
        }
    }

    int r0 = check_path(source_dir);
    int r1 = check_path(destiny_dir);

    // Si alguno de los paths es invalido termina la ejecución
    if (r0 || r1){
        return 1;
    }

    // Este TaskQueue será usado por el thread_pool y por el hilo principal
    TaskQueue q;
    if (init_queue(&q) != 0) {
        printf("Error iniciando la cola\n");
        return 1;
    }

    // se crea el pool, los hilos arrancan de una vez y quedan bloqueados en dequeue_wait esperando que lleguen tareas
    ThreadPool pool;
    if (thread_pool_init(&pool, num_threads, &q) != 0) {
        printf("Error iniciando el thread pool\n");
        finish_queue(&q);
        thread_pool_wait(&pool);
        thread_pool_destroy(&pool);
        queue_destroy(&q);
        return 1;
    }

    // obtener tiempo de ejecicion, para prubas de rendimiento
    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    // aca se empieza el recorrido del directorio y la creacion de tasks
    int r = traverse_source(source_dir, destiny_dir, &q);
    if (r){
        finish_queue(&q);
        thread_pool_wait(&pool);
        thread_pool_destroy(&pool);
        queue_destroy(&q);
        return 1;
    }

    // avisa a los hilos que ya no van a llegar mas tasks
    finish_queue(&q);

    // aqui espera a que todos los hilos procesen lo que queda y terminen
    // Ademas devuelve el worker info que cada uno recopiló
    WorkerInfo *workers_info = thread_pool_wait(&pool);

    // pruebas de rendimiento
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double total_time = (t_end.tv_sec - t_start.tv_sec) +
                        (t_end.tv_nsec - t_start.tv_nsec) / 1e9;
    printf("Tiempo total con %d hilo(s): %.6f segundos\n", num_threads, total_time);

    r = create_csv_log(workers_info, pool.num_threads);
    if (r){
        thread_pool_destroy(&pool);
        queue_destroy(&q);
        return 1;
    }

    thread_pool_destroy(&pool);
    queue_destroy(&q);

    return 0;
}
