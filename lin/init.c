#include "init.h"
#include "error.h"
#include "groupv2.h"
#include "storage.h"
#include "linio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void lin_cmd_execute_init(LinContext *ctx, int argc, int argvi, char **argv)
{
  (void)argc;
  (void)argvi;
  (void)argv;

  char lin_dir[PATH_MAX];
  char cwd[PATH_MAX];

  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    fprintf(stderr, "lin: failed to get working directory\n");
    exit(EXIT_FAILURE);
  }

  snprintf(lin_dir, sizeof(lin_dir), "%s/" LIN_DIR_NAME, cwd);

  if (lin_io_path_exists(lin_dir)) {
    fprintf(stdout, "reinitialized existing %s directory\n", LIN_DIR_NAME);
    return;
  }

  int rc = lin_storage_ensure_dir(lin_dir, 0755);
  if (rc != LIN_OK) {
    fprintf(stderr, "lin: failed to create %s: %s\n",
            LIN_DIR_NAME, lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  /* set up context so groupv2 can build paths */
  snprintf(ctx->root_dir, sizeof(ctx->root_dir), "%s", cwd);
  snprintf(ctx->lin_dir, sizeof(ctx->lin_dir), "%s", lin_dir);

  rc = lin_groupv2_create(ctx, "default");
  if (rc != LIN_OK && rc != LIN_ERR_GROUP_EXISTS) {
    fprintf(stderr, "lin: failed to create default group: %s\n",
            lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "initialized %s directory\n", LIN_DIR_NAME);
}
