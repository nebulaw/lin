#ifndef LIN_GROUP_H_
#define LIN_GROUP_H_

#include "linio.h"

#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <sys/stat.h>

#define GROUP_MAX_LEN 25

typedef struct {
    char group_name[GROUP_MAX_LEN]; // 25 bytes
    long long updated_ms;           // 8 bytes
    long long created_ms;           // 8 bytes
    long long total_lines;          // 8 bytes
    int total_files;                // 4 bytes
    int total_checkpoints;          // 4 bytes
    char root_node[PATH_MAX_LEN];   // 200 bytes
} LGroupInfo;                       // 257 bytes

void lin_cmd_execute_group(int argc, int argvi, char **argv);
int lin_group_create(const char *group_name);
int lin_group_remove(const char *group_name);
int lin_group_exists(const char *group_name);
int lin_group_info_create(const char *group_name);
int lin_group_info_update(const char *group_name, LGroupInfo info);

#endif // LIN_GROUP_H_