/* Filesytem Management

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "fs_mgmt.h"

int check_path(char *path)
{
    struct stat statbuf;

    if (stat(path, &statbuf) == -1) {
        perror("La ruta es incorrecta");
        return 1;
    }

    if (!S_ISDIR(statbuf.st_mode)) {
        printf("La ruta existe, pero no es un directorio\n");
        return 1;
    }

    return 0;
}


int traverse_source(char *base_path, char *dst_base_path, TaskQueue *q){

    struct dirent *dp;
    DIR *dir;

    if ((dir = opendir(base_path)) == NULL) {
        perror ("No se pudo abrir el directorio");
        return 1;
    }

    while ((dp = readdir(dir)) != NULL) {

        // Revisar que no sea "." ni ".."
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }

        // constuir el path
        Task new_task;
        snprintf(new_task.source_path, PATH_MAX, "%s/%s", base_path, dp->d_name);
        snprintf(new_task.dest_path, PATH_MAX, "%s/%s", dst_base_path, dp->d_name);

        printf("dp encontrado:%s\n", dp->d_name);
        printf("source path %s\n", new_task.source_path);
        printf("destiny path %s\n", new_task.dest_path);

        printf("\n");

        // Revisar que sea directorio o archivo
        struct stat statbuf;
        if (stat(new_task.source_path, &statbuf) == -1) {
            perror("Error leyendo entrada");
            closedir(dir);
            return 1;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // se crea el directorio destino aqui en el hilo principal, antes de encolar los archivos que van dentro
            // asi nunca se intenta copiar a una carpeta que no existe (no se copiaban los archivos dentro de sub carpetas)
            if (mkdir(new_task.dest_path, statbuf.st_mode) == -1) {
                perror("Error creando directorio destino");
                closedir(dir);
                return 1;
            }
            printf("[main] directorio creado: %s\n", new_task.dest_path);
            traverse_source(new_task.source_path, new_task.dest_path, q);
        } else if (S_ISREG(statbuf.st_mode)) {
            // solo se encolan archivos para que los workers los copien
            int status = enqueue(q, &new_task);
            if (status) {
                closedir(dir);
                return 1;
            }
        }

    }
    closedir(dir);
    return 0;
}
