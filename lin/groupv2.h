#ifndef LIN_GROUPV2_H
#define LIN_GROUPV2_H

#include "context.h"
#include "format.h"

#include <stdint.h>

#define LIN_GROUP_INFO_FILE "info"
#define LIN_GROUP_LIST_MAX  128

typedef struct {
  uint64_t created_ms;
  uint64_t updated_ms;
  uint64_t total_lines;
  uint32_t total_files;
  uint32_t total_checkpoints;
  char     name[LIN_GROUP_MAX + 1];
} LinGroupInfo;

int lin_groupv2_create(const LinContext *ctx, const char *name);
int lin_groupv2_remove(const LinContext *ctx, const char *name);
int lin_groupv2_exists(const LinContext *ctx, const char *name);

int lin_groupv2_info_read(const LinContext *ctx, const char *name,
                          LinGroupInfo *info);
int lin_groupv2_info_write(const LinContext *ctx, const char *name,
                           const LinGroupInfo *info);

/*
 * List all groups.  Returns the count, or negative on error.
 * Names are written into `names` as null-terminated strings.
 */
int lin_groupv2_list(const LinContext *ctx,
                     char names[][LIN_GROUP_MAX + 1], int max);

#endif /* LIN_GROUPV2_H */
