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

        // En caso de fallar el enqueue
        int status = enqueue(q, &new_task);
        if(status){
            return 1
        }

        printf("\n");

        // Revisar que sea directorio
        struct stat statbuf;
        if (stat(new_task.source_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            traverse_source(new_task.source_path, new_task.dest_path, q);
        }

    }
    closedir(dir);
    return 0;
}
