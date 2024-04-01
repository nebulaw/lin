#ifndef LIN_OBJECTS_H_
#define LIN_OBJECTS_H_

#include "linio.h"
#include "group.h"

#define OBJ_FILE_BUFFER_SIZE 1024
#define OBJ_FILE_MAX_LEN 200

typedef struct {
    int encoded_name[GROUP_MAX_LEN];
    long long updated_ms;
    long long created_ms;
    long long total_lines;
    int total_files;
    int total_checkpoints;
    int encoded_root_node[PATH_MAX_LEN];
} LGroupInfoObject;


int lin_obj_file_create(const char *file_path);
int lin_obj_file_remove(const char *file_path);

// related to group info
int lin_obj_write_group_info(const char *group_name, LGroupInfo *info);
int lin_obj_read_group_info(const char *group_name, LGroupInfo *info);

int lin_obj_write_object(const char *dst_path, void *object, size_t object_size);

#endif // LIN_OBJECTS_H_