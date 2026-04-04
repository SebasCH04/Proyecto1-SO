#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "task_queue.h"
#include "fs_mgmt.h"

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
    init_queue(&q);

    /*
     acá se debe crear el thread_pool y manderles el queue,
     basicamente lo que deben hacer es tratar de hacer dequeue_wait, y salir una vez ya se hayan
     terminado y se de la señal de finish_queue
     */

    // aca se empieza el recorrido del directorio y la creacion de tasks
    int r = traverse_source(source_dir, destiny_dir, &q);
    if (r){
        return 1;
    }
    // Este print se quita luego
    print_queue(&q);

    // Con esto se da por finalizada la cola,
    finish_queue(&q);
    // Ahora acá abajo toca hacer el wait a que los hilos terminen

    return 0;
}
