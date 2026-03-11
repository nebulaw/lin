#include "remove.h"
#include "error.h"
#include "manifest.h"
#include "groupv2.h"
#include "linio.h"

#include <stdio.h>
#include <stdlib.h>

void lin_cmd_execute_remove(LinContext *ctx, int argc, int argvi, char **argv)
{
  if (argvi >= argc) {
    fprintf(stderr, "lin: specify files to untrack\n");
    exit(EXIT_FAILURE);
  }

  LinGroupInfo info;
  int rc = lin_groupv2_info_read(ctx, ctx->group, &info);
  if (rc != LIN_OK) {
    fprintf(stderr, "lin: failed to read group info: %s\n",
            lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  int removed = 0;

  for (int i = argvi; i < argc; i++) {
    if (argv[i][0] == '-')
      continue;

    rc = lin_manifest_remove(ctx, ctx->group, argv[i]);
    if (rc == LIN_ERR_MANIFEST_ENTRY_NOT_FOUND) {
      fprintf(stderr, "lin: not tracked: %s\n", argv[i]);
    } else if (rc != LIN_OK) {
      fprintf(stderr, "lin: failed to remove %s: %s\n",
              argv[i], lin_error_get()->message);
    } else {
      if (ctx->verbose)
        fprintf(stdout, "  removed: %s\n", argv[i]);
      removed++;
    }
  }

  if (removed > 0) {
    if (info.total_files >= (uint32_t)removed)
      info.total_files -= (uint32_t)removed;
    else
      info.total_files = 0;

    info.updated_ms = lin_env_get_time();
    lin_groupv2_info_write(ctx, ctx->group, &info);
  }

  fprintf(stdout, "removed %d file(s) from '%s'\n", removed, ctx->group);
}
