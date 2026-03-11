#ifndef LIN_SNAP_H
#define LIN_SNAP_H

#include "context.h"
#include "manifest.h"

#include <stdint.h>

#define LIN_SNAP_DIR       "checkpoints"
#define LIN_SNAP_EXT       ".cp"
#define LIN_SNAP_ID_WIDTH  6  /* zero-padded width: 000001.cp */

typedef struct {
  char     sha1[LIN_SHA1_HEX_LEN + 1];
  uint64_t line_count;
  uint64_t file_size;
  uint16_t path_len;
  char     path[PATH_MAX];
} LinSnapEntry;

typedef struct {
  uint32_t id;
  uint64_t timestamp_ms;
  uint64_t total_lines;
  uint64_t total_size;
  uint32_t total_files;
  uint16_t message_len;
  char     message[LIN_MESSAGE_MAX + 1];
  LinSnapEntry *entries;    /* heap-allocated array */
  uint32_t entry_count;
} LinSnapshot;

/*
 * Create a new checkpoint by scanning all tracked files.
 * Writes a binary snapshot to the next sequential id.
 * Returns the new checkpoint id (> 0) or negative on error.
 */
int lin_snap_create(const LinContext *ctx, const char *group,
                    const char *message);

/*
 * Load a checkpoint by id.  Caller must call lin_snap_free()
 * on the result when done.
 */
int lin_snap_load(const LinContext *ctx, const char *group,
                  uint32_t id, LinSnapshot *snap);

/*
 * Find the highest checkpoint id in the group.
 * Returns 0 if no checkpoints exist.
 */
uint32_t lin_snap_latest_id(const LinContext *ctx, const char *group);

/*
 * List all checkpoint ids in ascending order.
 * Returns the count, or negative on error.
 */
int lin_snap_list(const LinContext *ctx, const char *group,
                  uint32_t *ids, int max_ids);

/*
 * Release heap memory held by a snapshot.
 */
void lin_snap_free(LinSnapshot *snap);

#endif /* LIN_SNAP_H */
