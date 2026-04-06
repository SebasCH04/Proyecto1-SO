#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "thread_pool.h"

#define STATS_FILE_NAME "logfile.csv"
// Este archivo utiliza el arreglo de workers_info y lo transforma en csv para posteriormente ser analizado
int create_csv_log(WorkerInfo *workers_info, int num_workers){

    FILE *f = fopen(STATS_FILE_NAME, "w");
    if (f == NULL){
        printf("Error creando el logfile\n");
        return 1;
    }

    // El encabezado del CSV
    if (fprintf(f, "worker_id,file_name,exec_time\n") < 0) {
        fclose(f);
        return 1;
    }


    for (int i = 0; i < num_workers; i++){

        // Ahora iterar en la lista de tareas realizadas
        for(int j = 0; j < workers_info[i].completed_count; j++ ){

            // al principio iba a usar el fwrite() que usa el profe
            // pero este fprintf lo simplifca bastante porque permite directamente escribir a un
            // archivo usando un formato
            if (fprintf(f, "\"%s\",\"%s\",%.6f\n", // se formatea asi para proteger de nombres de archivos con comas, puntos y comillas
                        workers_info[i].worker_id,
                        workers_info[i].completed_tasks[j].file_name,
                        workers_info[i].completed_tasks[j].exec_time) < 0) {
                fclose(f);
                return 1;
            }

        }

    }

    fclose(f);
    return 0;

}
