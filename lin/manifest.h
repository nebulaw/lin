#ifndef LIN_MANIFEST_H
#define LIN_MANIFEST_H

#include "context.h"

#include <limits.h>
#include <stdint.h>

#define LIN_MANIFEST_HEADER  "#lin-manifest-v1"
#define LIN_MANIFEST_FILE    "tracked"
#define LIN_SHA1_HEX_LEN    40
#define LIN_MANIFEST_MAX     4096

typedef struct {
  char     sha1[LIN_SHA1_HEX_LEN + 1];
  uint64_t line_count;
  uint64_t file_size;
  uint64_t added_ms;
  char     path[PATH_MAX];
} LinManifestEntry;

/*
 * Load all entries from the group's manifest file.
 * Returns the number of entries read, or negative on error.
 */
int lin_manifest_load(const LinContext *ctx, const char *group,
                      LinManifestEntry *entries, int max_entries);

/*
 * Save `count` entries to the group's manifest file (atomic write).
 * Overwrites the entire manifest.
 */
int lin_manifest_save(const LinContext *ctx, const char *group,
                      const LinManifestEntry *entries, int count);

/*
 * Append a single entry.  Returns error if path already tracked.
 */
int lin_manifest_append(const LinContext *ctx, const char *group,
                        const LinManifestEntry *entry);

/*
 * Remove entry by file path.
 */
int lin_manifest_remove(const LinContext *ctx, const char *group,
                        const char *file_path);

/*
 * Find entry by file path.  Returns index (>= 0) or negative on error.
 * If `out` is non-NULL, copies the entry into it.
 */
int lin_manifest_find(const LinContext *ctx, const char *group,
                      const char *file_path, LinManifestEntry *out);

/*
 * Update an existing entry (matched by path).
 */
int lin_manifest_update(const LinContext *ctx, const char *group,
                        const LinManifestEntry *entry);

#endif /* LIN_MANIFEST_H */
