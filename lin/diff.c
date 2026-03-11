#include "diff.h"
#include "error.h"
#include "snap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void lin_cmd_execute_diff(LinContext *ctx, int argc, int argvi, char **argv)
{
  if (argvi + 1 >= argc) {
    fprintf(stderr, "usage: lin diff <checkpoint1> <checkpoint2>\n");
    exit(EXIT_FAILURE);
  }

  /* skip flags */
  while (argvi < argc && argv[argvi][0] == '-')
    argvi++;

  if (argvi + 1 >= argc) {
    fprintf(stderr, "usage: lin diff <checkpoint1> <checkpoint2>\n");
    exit(EXIT_FAILURE);
  }

  uint32_t id1 = (uint32_t)atoi(argv[argvi]);
  uint32_t id2 = (uint32_t)atoi(argv[argvi + 1]);

  if (id1 == 0 || id2 == 0) {
    fprintf(stderr, "lin: checkpoint ids must be positive integers\n");
    exit(EXIT_FAILURE);
  }

  LinSnapshot snap1, snap2;

  int rc = lin_snap_load(ctx, ctx->group, id1, &snap1);
  if (rc != LIN_OK) {
    fprintf(stderr, "lin: cannot load checkpoint #%u: %s\n",
            id1, lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  rc = lin_snap_load(ctx, ctx->group, id2, &snap2);
  if (rc != LIN_OK) {
    fprintf(stderr, "lin: cannot load checkpoint #%u: %s\n",
            id2, lin_error_get()->message);
    lin_snap_free(&snap1);
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "diff #%u -> #%u\n\n", id1, id2);

  int n_added   = 0;
  int n_removed = 0;
  int n_changed = 0;

  /* find files in snap2 that are new or changed */
  for (uint32_t i = 0; i < snap2.entry_count; i++) {
    int found = 0;
    for (uint32_t j = 0; j < snap1.entry_count; j++) {
      if (strcmp(snap2.entries[i].path, snap1.entries[j].path) == 0) {
        found = 1;
        if (strcmp(snap2.entries[i].sha1, snap1.entries[j].sha1) != 0) {
          int64_t dl = (int64_t)snap2.entries[i].line_count -
                       (int64_t)snap1.entries[j].line_count;
          fprintf(stdout, "  modified: %s (%+lld lines)\n",
                  snap2.entries[i].path, (long long)dl);
          n_changed++;
        }
        break;
      }
    }
    if (!found) {
      fprintf(stdout, "  added:    %s (%llu lines)\n",
              snap2.entries[i].path,
              (unsigned long long)snap2.entries[i].line_count);
      n_added++;
    }
  }

  /* find files in snap1 not in snap2 */
  for (uint32_t i = 0; i < snap1.entry_count; i++) {
    int found = 0;
    for (uint32_t j = 0; j < snap2.entry_count; j++) {
      if (strcmp(snap1.entries[i].path, snap2.entries[j].path) == 0) {
        found = 1;
        break;
      }
    }
    if (!found) {
      fprintf(stdout, "  removed:  %s (%llu lines)\n",
              snap1.entries[i].path,
              (unsigned long long)snap1.entries[i].line_count);
      n_removed++;
    }
  }

  /* summary */
  int64_t dl = (int64_t)snap2.total_lines - (int64_t)snap1.total_lines;
  int64_t ds = (int64_t)snap2.total_size  - (int64_t)snap1.total_size;

  fprintf(stdout, "\nsummary:\n");
  fprintf(stdout, "  %d added, %d removed, %d modified\n",
          n_added, n_removed, n_changed);
  fprintf(stdout, "  lines: %llu -> %llu (%+lld)\n",
          (unsigned long long)snap1.total_lines,
          (unsigned long long)snap2.total_lines,
          (long long)dl);
  fprintf(stdout, "  size:  %llu -> %llu (%+lld bytes)\n",
          (unsigned long long)snap1.total_size,
          (unsigned long long)snap2.total_size,
          (long long)ds);

  lin_snap_free(&snap1);
  lin_snap_free(&snap2);
}
