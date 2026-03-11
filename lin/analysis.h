#ifndef LIN_ANALYSIS_H
#define LIN_ANALYSIS_H

#include "manifest.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  char     sha1[LIN_SHA1_HEX_LEN + 1];
  uint64_t line_count;
  uint64_t file_size;
  bool     exists;
  bool     is_binary;
} LinFileStats;

/*
 * Scan a single file in one pass: compute SHA1 hash,
 * count lines, and get file size.
 */
int lin_analysis_scan_file(const char *path, LinFileStats *stats);

#endif /* LIN_ANALYSIS_H */
