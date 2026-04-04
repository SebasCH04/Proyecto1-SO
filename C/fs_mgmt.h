#ifndef FS_MGMT_H
#define FS_MGMT_H
#include "task_queue.h"

int check_path(char *path);

int traverse_source(char *base_path, char *dst_base_path, TaskQueue *q );

#endif
