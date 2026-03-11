#include "log.h"
#include "error.h"
#include "snap.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void format_timestamp(uint64_t ms, char *buf, size_t size)
{
  time_t sec = (time_t)(ms / 1000);
  struct tm *tm = localtime(&sec);
  if (tm != NULL)
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm);
  else
    snprintf(buf, size, "%llu", (unsigned long long)ms);
}

void lin_cmd_execute_log(LinContext *ctx, int argc, int argvi, char **argv)
{
  (void)argc;
  (void)argvi;
  (void)argv;

  uint32_t ids[4096];
  int count = lin_snap_list(ctx, ctx->group, ids, 4096);

  if (count < 0) {
    fprintf(stderr, "lin: failed to list checkpoints: %s\n",
            lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  if (count == 0) {
    fprintf(stdout, "no checkpoints in '%s'\n", ctx->group);
    return;
  }

  uint64_t prev_lines = 0;

  for (int i = 0; i < count; i++) {
    LinSnapshot snap;
    int rc = lin_snap_load(ctx, ctx->group, ids[i], &snap);
    if (rc != LIN_OK) {
      fprintf(stderr, "  #%u: unreadable\n", ids[i]);
      continue;
    }

    char ts[32];
    format_timestamp(snap.timestamp_ms, ts, sizeof(ts));

    int64_t delta = (int64_t)snap.total_lines - (int64_t)prev_lines;

    fprintf(stdout, "#%-6u  %s  %5u files  %7llu lines",
            snap.id, ts, snap.total_files,
            (unsigned long long)snap.total_lines);

    if (i > 0)
      fprintf(stdout, "  %+lld", (long long)delta);

    if (snap.message_len > 0)
      fprintf(stdout, "  \"%s\"", snap.message);

    fprintf(stdout, "\n");

    if (ctx->verbose) {
      for (uint32_t j = 0; j < snap.entry_count; j++) {
        fprintf(stdout, "           %7llu  %s\n",
                (unsigned long long)snap.entries[j].line_count,
                snap.entries[j].path);
      }
      fprintf(stdout, "\n");
    }

    prev_lines = snap.total_lines;
    lin_snap_free(&snap);
  }
}
