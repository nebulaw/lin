#include "group.h"
#include "error.h"
#include "groupv2.h"
#include "linio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void group_create(LinContext *ctx, int argc, int argvi, char **argv)
{
  if (argvi >= argc) {
    fprintf(stderr, "lin: specify group name\n");
    exit(EXIT_FAILURE);
  }

  const char *name = argv[argvi];
  int rc = lin_groupv2_create(ctx, name);

  switch (rc) {
  case LIN_OK:
    fprintf(stdout, "created group '%s'\n", name);
    break;
  case LIN_ERR_GROUP_EXISTS:
    fprintf(stdout, "lin: group '%s' already exists\n", name);
    break;
  default:
    fprintf(stderr, "lin: failed to create group '%s': %s\n",
            name, lin_error_get()->message);
    exit(EXIT_FAILURE);
  }
}

static void group_remove(LinContext *ctx, int argc, int argvi, char **argv)
{
  if (argvi >= argc) {
    fprintf(stderr, "lin: specify group name\n");
    exit(EXIT_FAILURE);
  }

  const char *name = argv[argvi];

  if (strcmp(name, "default") == 0) {
    fprintf(stderr, "lin: cannot remove the default group\n");
    exit(EXIT_FAILURE);
  }

  int rc = lin_groupv2_remove(ctx, name);
  if (rc == LIN_ERR_GROUP_NOT_FOUND) {
    fprintf(stderr, "lin: group '%s' not found\n", name);
    exit(EXIT_FAILURE);
  } else if (rc != LIN_OK) {
    fprintf(stderr, "lin: failed to remove '%s': %s\n",
            name, lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "removed group '%s'\n", name);
}

static void group_list(LinContext *ctx)
{
  char names[LIN_GROUP_LIST_MAX][LIN_GROUP_MAX + 1];
  int count = lin_groupv2_list(ctx, names, LIN_GROUP_LIST_MAX);

  if (count < 0) {
    fprintf(stderr, "lin: failed to list groups: %s\n",
            lin_error_get()->message);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < count; i++) {
    LinGroupInfo info;
    int rc = lin_groupv2_info_read(ctx, names[i], &info);

    if (rc == LIN_OK) {
      fprintf(stdout, "  %-20s  %u files  %llu lines  %u checkpoints\n",
              names[i],
              info.total_files,
              (unsigned long long)info.total_lines,
              info.total_checkpoints);
    } else {
      fprintf(stdout, "  %-20s  (unreadable info)\n", names[i]);
    }
  }

  if (count == 0)
    fprintf(stdout, "no groups found\n");
}

void lin_cmd_execute_group(LinContext *ctx, int argc, int argvi, char **argv)
{
  if (argvi >= argc) {
    fprintf(stderr, "lin: specify subcommand (create, remove, list)\n");
    fprintf(stderr, "type 'lin help group' for more\n");
    exit(EXIT_FAILURE);
  }

  const char *sub = argv[argvi];
  argvi++;

  if (strcmp(sub, "create") == 0 || strcmp(sub, "mk") == 0) {
    group_create(ctx, argc, argvi, argv);
  } else if (strcmp(sub, "remove") == 0 || strcmp(sub, "rm") == 0) {
    group_remove(ctx, argc, argvi, argv);
  } else if (strcmp(sub, "list") == 0 || strcmp(sub, "ls") == 0) {
    group_list(ctx);
  } else {
    fprintf(stderr, "lin: unknown subcommand: %s\n", sub);
    exit(EXIT_FAILURE);
  }
}
