#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "task_queue.h"
#include "fs_mgmt.h"
#include "thread_pool.h"
#include "logging.h"

#define NUM_THREADS 4

int main(int argc, char *argv[]){

    if (argc != 3){
        printf("Se debe pasar ambos parametros: cp source_dir destiny_dir\n");
        return 1;
    }

    char *source_dir = argv[1];
    char *destiny_dir = argv[2];

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
    if (thread_pool_init(&pool, NUM_THREADS, &q) != 0) {
        printf("Error iniciando el thread pool\n");
        finish_queue(&q);
        thread_pool_wait(&pool);
        thread_pool_destroy(&pool);
        queue_destroy(&q);
        return 1;
    }

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
