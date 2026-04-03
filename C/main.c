#include <stdio.h>
#include <string.h>
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

    int r2 = traverse_source(source_dir);

    return 0;
}
