#include "checkpoint.h"
#include "error.h"
#include "groupv2.h"
#include "snap.h"

#include <stdio.h>
#include <stdlib.h>

void lin_cmd_execute_checkpoint(LinContext *ctx, int argc, int argvi,
                                char **argv)
{
  (void)argc;
  (void)argvi;
  (void)argv;

  const char *message = ctx->message_set ? ctx->message : NULL;

  /* load previous state for delta reporting */
  uint32_t prev_id = lin_snap_latest_id(ctx, ctx->group);
  uint64_t prev_lines = 0;
  uint32_t prev_files = 0;

  if (prev_id > 0) {
    LinSnapshot prev;
    if (lin_snap_load(ctx, ctx->group, prev_id, &prev) == LIN_OK) {
      prev_lines = prev.total_lines;
      prev_files = prev.total_files;
      lin_snap_free(&prev);
    }
  }

  /* create snapshot */
  int result = lin_snap_create(ctx, ctx->group, message);
  if (result < 0) {
    fprintf(stderr, "lin: checkpoint failed: %s\n",
            lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  uint32_t new_id = (uint32_t)result;

  /* load the snapshot we just wrote for reporting */
  LinSnapshot snap;
  int rc = lin_snap_load(ctx, ctx->group, new_id, &snap);
  if (rc != LIN_OK) {
    fprintf(stderr, "lin: failed to read checkpoint: %s\n",
            lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  /* update group info */
  LinGroupInfo info;
  rc = lin_groupv2_info_read(ctx, ctx->group, &info);
  if (rc == LIN_OK) {
    info.total_lines       = snap.total_lines;
    info.total_checkpoints = new_id;
    lin_groupv2_info_write(ctx, ctx->group, &info);
  }

  /* report */
  fprintf(stdout, "checkpoint #%u", new_id);
  if (message != NULL)
    fprintf(stdout, " \"%s\"", message);
  fprintf(stdout, "\n");

  fprintf(stdout, "  files: %u", snap.total_files);
  if (prev_id > 0) {
    int32_t df = (int32_t)snap.total_files - (int32_t)prev_files;
    if (df != 0)
      fprintf(stdout, " (%+d)", df);
  }
  fprintf(stdout, "\n");

  fprintf(stdout, "  lines: %llu", (unsigned long long)snap.total_lines);
  if (prev_id > 0) {
    int64_t dl = (int64_t)snap.total_lines - (int64_t)prev_lines;
    if (dl != 0)
      fprintf(stdout, " (%+lld)", (long long)dl);
  }
  fprintf(stdout, "\n");

  fprintf(stdout, "  size:  %llu bytes\n",
          (unsigned long long)snap.total_size);

  /* verbose: per-file details */
  if (ctx->verbose) {
    fprintf(stdout, "\n");
    for (uint32_t i = 0; i < snap.entry_count; i++) {
      fprintf(stdout, "  %7llu  %s\n",
              (unsigned long long)snap.entries[i].line_count,
              snap.entries[i].path);
    }
  }

  lin_snap_free(&snap);
}
