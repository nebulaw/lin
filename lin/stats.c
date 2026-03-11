#include "stats.h"
#include "error.h"
#include "groupv2.h"
#include "manifest.h"
#include "snap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_EXTENSIONS 128
#define EXT_MAX_LEN    16

typedef struct {
  char     ext[EXT_MAX_LEN];
  uint64_t lines;
  uint32_t files;
} ExtStats;

static const char *get_extension(const char *path)
{
  const char *dot = strrchr(path, '.');
  const char *sep = strrchr(path, '/');

  if (dot == NULL || (sep != NULL && dot < sep))
    return NULL;

  return dot + 1;
}

static void format_timestamp(uint64_t ms, char *buf, size_t size)
{
  time_t sec = (time_t)(ms / 1000);
  struct tm *tm = localtime(&sec);
  if (tm != NULL)
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm);
  else
    snprintf(buf, size, "%llu", (unsigned long long)ms);
}

void lin_cmd_execute_stats(LinContext *ctx, int argc, int argvi, char **argv)
{
  (void)argc;
  (void)argvi;
  (void)argv;

  /* group info */
  LinGroupInfo info;
  int rc = lin_groupv2_info_read(ctx, ctx->group, &info);
  if (rc != LIN_OK) {
    fprintf(stderr, "lin: failed to read group info: %s\n",
            lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  char created[32], updated[32];
  format_timestamp(info.created_ms, created, sizeof(created));
  format_timestamp(info.updated_ms, updated, sizeof(updated));

  fprintf(stdout, "group: %s\n", ctx->group);
  fprintf(stdout, "  created:     %s\n", created);
  fprintf(stdout, "  updated:     %s\n", updated);
  fprintf(stdout, "  files:       %u\n", info.total_files);
  fprintf(stdout, "  lines:       %llu\n", (unsigned long long)info.total_lines);
  fprintf(stdout, "  checkpoints: %u\n", info.total_checkpoints);

  /* extension breakdown from manifest */
  LinManifestEntry *entries = malloc(sizeof(LinManifestEntry) * LIN_MANIFEST_MAX);
  if (entries == NULL)
    return;

  int file_count = lin_manifest_load(ctx, ctx->group, entries, LIN_MANIFEST_MAX);
  if (file_count > 0) {
    ExtStats exts[MAX_EXTENSIONS];
    int ext_count = 0;

    for (int i = 0; i < file_count; i++) {
      const char *ext = get_extension(entries[i].path);
      if (ext == NULL)
        ext = "(none)";

      int found = -1;
      for (int j = 0; j < ext_count; j++) {
        if (strcmp(exts[j].ext, ext) == 0) {
          found = j;
          break;
        }
      }

      if (found >= 0) {
        exts[found].lines += entries[i].line_count;
        exts[found].files++;
      } else if (ext_count < MAX_EXTENSIONS) {
        snprintf(exts[ext_count].ext, EXT_MAX_LEN, "%s", ext);
        exts[ext_count].lines = entries[i].line_count;
        exts[ext_count].files = 1;
        ext_count++;
      }
    }

    /* sort by lines descending */
    for (int i = 0; i < ext_count - 1; i++) {
      for (int j = i + 1; j < ext_count; j++) {
        if (exts[j].lines > exts[i].lines) {
          ExtStats tmp = exts[i];
          exts[i] = exts[j];
          exts[j] = tmp;
        }
      }
    }

    fprintf(stdout, "\nby extension:\n");
    for (int i = 0; i < ext_count; i++) {
      fprintf(stdout, "  .%-10s  %5u files  %7llu lines\n",
              exts[i].ext, exts[i].files,
              (unsigned long long)exts[i].lines);
    }
  }

  /* checkpoint growth trend */
  uint32_t ids[4096];
  int snap_count = lin_snap_list(ctx, ctx->group, ids, 4096);

  if (snap_count > 1) {
    fprintf(stdout, "\ngrowth trend:\n");

    uint64_t prev_lines = 0;
    int64_t total_delta = 0;

    for (int i = 0; i < snap_count; i++) {
      LinSnapshot snap;
      rc = lin_snap_load(ctx, ctx->group, ids[i], &snap);
      if (rc != LIN_OK)
        continue;

      if (i > 0) {
        int64_t delta = (int64_t)snap.total_lines - (int64_t)prev_lines;
        total_delta += delta;

        fprintf(stdout, "  #%-4u -> #%-4u  %+lld lines\n",
                ids[i - 1], ids[i], (long long)delta);
      }

      prev_lines = snap.total_lines;
      lin_snap_free(&snap);
    }

    if (snap_count > 1) {
      int64_t avg = total_delta / (snap_count - 1);
      fprintf(stdout, "  average: %+lld lines/checkpoint\n", (long long)avg);
    }
  }

  free(entries);
}
