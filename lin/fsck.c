#include "fsck.h"
#include "error.h"
#include "format.h"
#include "groupv2.h"
#include "manifest.h"
#include "snap.h"
#include "linio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check_group_info(const LinContext *ctx, const char *group,
                            int *errors)
{
  LinGroupInfo info;
  int rc = lin_groupv2_info_read(ctx, group, &info);
  if (rc != LIN_OK) {
    fprintf(stderr, "  error: cannot read info file: %s\n",
            lin_error_get()->message);
    (*errors)++;
    return rc;
  }

  fprintf(stdout, "  info: ok (magic valid, version %d)\n", LIN_FORMAT_V1);
  return LIN_OK;
}

static int check_manifest(const LinContext *ctx, const char *group,
                          int *errors, int *warnings)
{
  LinManifestEntry *entries = malloc(sizeof(LinManifestEntry) * LIN_MANIFEST_MAX);
  if (entries == NULL)
    return LIN_ERR_NOMEM;

  int count = lin_manifest_load(ctx, group, entries, LIN_MANIFEST_MAX);
  if (count < 0) {
    fprintf(stderr, "  error: manifest unreadable: %s\n",
            lin_error_get()->message);
    (*errors)++;
    free(entries);
    return count;
  }

  fprintf(stdout, "  manifest: %d entries\n", count);

  for (int i = 0; i < count; i++) {
    if (!lin_io_path_exists(entries[i].path)) {
      fprintf(stdout, "    warning: missing file: %s\n", entries[i].path);
      (*warnings)++;
    }
  }

  free(entries);
  return LIN_OK;
}

static int check_checkpoints(const LinContext *ctx, const char *group,
                             int *errors)
{
  uint32_t ids[4096];
  int count = lin_snap_list(ctx, group, ids, 4096);

  if (count < 0) {
    fprintf(stderr, "  error: cannot list checkpoints: %s\n",
            lin_error_get()->message);
    (*errors)++;
    return count;
  }

  fprintf(stdout, "  checkpoints: %d found\n", count);

  for (int i = 0; i < count; i++) {
    LinSnapshot snap;
    int rc = lin_snap_load(ctx, group, ids[i], &snap);
    if (rc != LIN_OK) {
      fprintf(stderr, "    error: checkpoint #%u corrupt: %s\n",
              ids[i], lin_error_get()->message);
      (*errors)++;
    } else {
      fprintf(stdout, "    #%u: ok (%u files, %llu lines)\n",
              ids[i], snap.total_files,
              (unsigned long long)snap.total_lines);
      lin_snap_free(&snap);
    }
  }

  return LIN_OK;
}

void lin_cmd_execute_fsck(LinContext *ctx, int argc, int argvi, char **argv)
{
  (void)argc;
  (void)argvi;
  (void)argv;

  int total_errors   = 0;
  int total_warnings = 0;

  /* determine which groups to check */
  char names[LIN_GROUP_LIST_MAX][LIN_GROUP_MAX + 1];
  int group_count;

  if (ctx->group_set) {
    snprintf(names[0], sizeof(names[0]), "%s", ctx->group);
    group_count = 1;
  } else {
    group_count = lin_groupv2_list(ctx, names, LIN_GROUP_LIST_MAX);
    if (group_count < 0) {
      fprintf(stderr, "lin: cannot list groups\n");
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < group_count; i++) {
    fprintf(stdout, "checking '%s'...\n", names[i]);

    check_group_info(ctx, names[i], &total_errors);
    check_manifest(ctx, names[i], &total_errors, &total_warnings);
    check_checkpoints(ctx, names[i], &total_errors);

    fprintf(stdout, "\n");
  }

  if (total_errors == 0 && total_warnings == 0) {
    fprintf(stdout, "all checks passed\n");
  } else {
    fprintf(stdout, "%d error(s), %d warning(s)\n",
            total_errors, total_warnings);
    if (total_errors > 0)
      exit(EXIT_FAILURE);
  }
}
