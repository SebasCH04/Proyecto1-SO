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


int traverse_source(char *base_path){

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

        // Construct full path
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", base_path, dp->d_name);
        printf("%s\n", path);

        // Revisar que sea directorio
        struct stat statbuf;
        if (stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            printf("Directory: %s\n", path);
            traverse_source(path);
        }

    }
    closedir(dir);
    return 0;
}
