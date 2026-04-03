/* Filesytem Management

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
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
