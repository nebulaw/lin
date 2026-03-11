#include "status.h"
#include "analysis.h"
#include "error.h"
#include "manifest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void lin_cmd_execute_status(LinContext *ctx, int argc, int argvi, char **argv)
{
  (void)argc;
  (void)argvi;
  (void)argv;

  LinManifestEntry *entries = malloc(sizeof(LinManifestEntry) * LIN_MANIFEST_MAX);
  if (entries == NULL) {
    fprintf(stderr, "lin: out of memory\n");
    exit(EXIT_FAILURE);
  }

  int count = lin_manifest_load(ctx, ctx->group, entries, LIN_MANIFEST_MAX);
  if (count < 0) {
    fprintf(stderr, "lin: failed to load manifest: %s\n",
            lin_error_get()->message);
    free(entries);
    exit(EXIT_FAILURE);
  }

  if (count == 0) {
    fprintf(stdout, "no files tracked in '%s'\n", ctx->group);
    free(entries);
    return;
  }

  int n_unchanged = 0;
  int n_modified  = 0;
  int n_missing   = 0;
  uint64_t current_lines = 0;
  uint64_t tracked_lines = 0;

  for (int i = 0; i < count; i++) {
    LinFileStats fs;
    int rc = lin_analysis_scan_file(entries[i].path, &fs);

    tracked_lines += entries[i].line_count;

    if (rc != LIN_OK) {
      n_missing++;
      if (ctx->verbose)
        fprintf(stdout, "  missing:   %s\n", entries[i].path);
      continue;
    }

    current_lines += fs.line_count;

    if (strcmp(fs.sha1, entries[i].sha1) == 0) {
      n_unchanged++;
      if (ctx->verbose)
        fprintf(stdout, "  unchanged: %s (%llu lines)\n",
                entries[i].path, (unsigned long long)fs.line_count);
    } else {
      n_modified++;
      int64_t delta = (int64_t)fs.line_count - (int64_t)entries[i].line_count;
      if (ctx->verbose)
        fprintf(stdout, "  modified:  %s (%llu -> %llu lines, %+lld)\n",
                entries[i].path,
                (unsigned long long)entries[i].line_count,
                (unsigned long long)fs.line_count,
                (long long)delta);
    }
  }

  fprintf(stdout, "\ngroup '%s': %d tracked\n", ctx->group, count);

  if (n_modified > 0)
    fprintf(stdout, "  %d modified\n", n_modified);
  if (n_missing > 0)
    fprintf(stdout, "  %d missing\n", n_missing);
  if (n_unchanged > 0)
    fprintf(stdout, "  %d unchanged\n", n_unchanged);

  if (n_modified > 0 || n_missing > 0) {
    int64_t dl = (int64_t)current_lines - (int64_t)tracked_lines;
    fprintf(stdout, "  lines: %llu tracked, %llu current (%+lld)\n",
            (unsigned long long)tracked_lines,
            (unsigned long long)current_lines,
            (long long)dl);
  }

  free(entries);
}
