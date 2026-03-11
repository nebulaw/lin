#include "add.h"
#include "analysis.h"
#include "error.h"
#include "groupv2.h"
#include "ignore.h"
#include "manifest.h"
#include "linio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void lin_cmd_execute_add(LinContext *ctx, int argc, int argvi, char **argv)
{
  if (argvi >= argc) {
    fprintf(stderr, "lin: specify files to track\n");
    exit(EXIT_FAILURE);
  }

  LinIgnore ign;
  lin_ignore_load(ctx, &ign);

  /* validate all paths first */
  for (int i = argvi; i < argc; i++) {
    if (argv[i][0] == '-')
      continue;
    if (!lin_io_path_exists(argv[i])) {
      fprintf(stderr, "lin: file not found: %s\n", argv[i]);
      exit(EXIT_FAILURE);
    }
  }

  LinGroupInfo info;
  int rc = lin_groupv2_info_read(ctx, ctx->group, &info);
  if (rc != LIN_OK) {
    fprintf(stderr, "lin: failed to read group info: %s\n",
            lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  int added   = 0;
  int updated = 0;
  int skipped = 0;

  for (int i = argvi; i < argc; i++) {
    if (argv[i][0] == '-')
      continue;

    /* check ignore rules */
    if (lin_ignore_match(&ign, argv[i])) {
      if (ctx->verbose)
        fprintf(stdout, "  ignored: %s\n", argv[i]);
      skipped++;
      continue;
    }

    /* scan file: hash + lines + size in one pass */
    LinFileStats fs;
    rc = lin_analysis_scan_file(argv[i], &fs);
    if (rc != LIN_OK) {
      fprintf(stderr, "lin: cannot scan %s: %s\n",
              argv[i], lin_error_get()->message);
      continue;
    }

    if (fs.is_binary && ctx->verbose)
      fprintf(stdout, "  warning: %s appears to be binary\n", argv[i]);

    /* check if already tracked */
    LinManifestEntry existing;
    int found = lin_manifest_find(ctx, ctx->group, argv[i], &existing);

    if (found >= 0) {
      /* already tracked: update if hash changed */
      if (strcmp(existing.sha1, fs.sha1) == 0) {
        if (ctx->verbose)
          fprintf(stdout, "  unchanged: %s\n", argv[i]);
        skipped++;
        continue;
      }

      existing.line_count = fs.line_count;
      existing.file_size  = fs.file_size;
      memcpy(existing.sha1, fs.sha1, LIN_SHA1_HEX_LEN + 1);

      rc = lin_manifest_update(ctx, ctx->group, &existing);
      if (rc != LIN_OK) {
        fprintf(stderr, "lin: failed to update %s: %s\n",
                argv[i], lin_error_get()->message);
        continue;
      }

      if (ctx->verbose)
        fprintf(stdout, "  updated: %s (%llu lines)\n",
                argv[i], (unsigned long long)fs.line_count);
      updated++;
    } else {
      /* new file: append to manifest */
      LinManifestEntry entry;
      memset(&entry, 0, sizeof(entry));
      memcpy(entry.sha1, fs.sha1, LIN_SHA1_HEX_LEN + 1);
      entry.line_count = fs.line_count;
      entry.file_size  = fs.file_size;
      entry.added_ms   = lin_env_get_time();
      snprintf(entry.path, sizeof(entry.path), "%s", argv[i]);

      rc = lin_manifest_append(ctx, ctx->group, &entry);
      if (rc != LIN_OK) {
        fprintf(stderr, "lin: failed to add %s: %s\n",
                argv[i], lin_error_get()->message);
        continue;
      }

      if (ctx->verbose)
        fprintf(stdout, "  added: %s (%llu lines)\n",
                argv[i], (unsigned long long)fs.line_count);
      added++;
    }
  }

  /* update group info */
  info.total_files += (uint32_t)added;
  info.updated_ms   = lin_env_get_time();
  lin_groupv2_info_write(ctx, ctx->group, &info);

  fprintf(stdout, "%d added, %d updated, %d unchanged in '%s'\n",
          added, updated, skipped, ctx->group);
}
