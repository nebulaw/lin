#ifndef LIN_GROUP_H_
#define LIN_GROUP_H_

#define _XOPEN_SOURCE 500

#include <limits.h>
#include <unistd.h>


typedef struct
{
    unsigned long updated_ms;       // 8 bytes
    unsigned long created_ms;       // 8 bytes
    unsigned long total_lines;      // 8 bytes
    int total_files;                // 4 bytes
    int total_checkpoints;          // 4 bytes
} LGroupInfo;                     // 32 bytes

void lin_cmd_execute_group(int argc, int argvi, char **argv);

int lin_group_create(const char *group_name);

int lin_group_remove(const char *group_name);

int lin_group_exists(const char *group_name);

int lin_group_info_create(const char *group_name);

int lin_group_info_update(const char *group_name, LGroupInfo info);

#endif // LIN_GROUP_H_
