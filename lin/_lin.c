#include "context.h"
#include "error.h"
#include "groupv2.h"
#include "help.h"

#include "add.h"
#include "checkpoint.h"
#include "diff.h"
#include "fsck.h"
#include "group.h"
#include "init.h"
#include "log.h"
#include "remove.h"
#include "stats.h"
#include "status.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIN_VERSION "0.2.0"

typedef void (*lin_cmd_fn)(LinContext *ctx, int argc, int argvi, char **argv);

struct LinCmdEntry {
  const char  *cmd;
  const char  *alias;
  lin_cmd_fn   fn;
};

static struct LinCmdEntry cmd_map[] = {
    { "help",       "h",   lin_cmd_execute_help },
    { "init",       NULL,  lin_cmd_execute_init },
    { "group",      NULL,  lin_cmd_execute_group },
    { "add",        "a",   lin_cmd_execute_add },
    { "remove",     "rm",  lin_cmd_execute_remove },
    { "checkpoint", "cp",  lin_cmd_execute_checkpoint },
    { "status",     "st",  lin_cmd_execute_status },
    { "log",        NULL,  lin_cmd_execute_log },
    { "stats",      NULL,  lin_cmd_execute_stats },
    { "diff",       NULL,  lin_cmd_execute_diff },
    { "fsck",       NULL,  lin_cmd_execute_fsck },
    { NULL, NULL, NULL },
};

static struct option arg_options[] = {
    { "group",   required_argument, 0, 'g' },
    { "message", required_argument, 0, 'm' },
    { "verbose", no_argument,       0, 'v' },
    { "version", no_argument,       0, 'V' },
    { "help",    no_argument,       0, 'H' },
    { 0, 0, 0, 0 },
};

int main(int argc, char **argv)
{
  LinContext ctx;
  lin_context_init(&ctx);

  if (argc < 2) {
    fprintf(stdout, "lin: no command specified\n");
    lin_help_print();
    return EXIT_FAILURE;
  }

  /* handle --version / --help before command parsing */
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-V") == 0) {
      fprintf(stdout, "lin %s\n", LIN_VERSION);
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[i], "--help") == 0 && i == 1) {
      lin_help_print();
      return EXIT_SUCCESS;
    }
  }

  /* find command name (skip leading flags) */
  int cmd_pos = 1;
  while (cmd_pos < argc && argv[cmd_pos][0] == '-')
    cmd_pos++;

  if (cmd_pos >= argc) {
    fprintf(stderr, "lin: no command specified\n");
    return EXIT_FAILURE;
  }

  char *command = argv[cmd_pos];

  /* parse global flags */
  int op;
  optind = 1;
  while ((op = getopt_long(argc, argv, "m:g:vVH", arg_options, NULL)) != -1) {
    switch (op) {
    case 'g':
      if (lin_context_set_group(&ctx, optarg) != LIN_OK) {
        fprintf(stderr, "lin: %s\n", lin_error_get()->message);
        return EXIT_FAILURE;
      }
      break;
    case 'v':
      ctx.verbose = true;
      break;
    case 'm':
      if (lin_context_set_message(&ctx, optarg) != LIN_OK) {
        fprintf(stderr, "lin: %s\n", lin_error_get()->message);
        return EXIT_FAILURE;
      }
      break;
    case 'V':
      fprintf(stdout, "lin %s\n", LIN_VERSION);
      return EXIT_SUCCESS;
    case 'H':
      lin_help_print();
      return EXIT_SUCCESS;
    case '?':
      return EXIT_FAILURE;
    default:
      break;
    }
  }

  /* re-find command position after getopt reordering */
  int argvi = 0;
  while (argvi < argc && strcmp(argv[argvi], command) != 0)
    argvi++;

  /* commands that need .lin to exist */
  int needs_root = (strcmp(command, "init") != 0 &&
                    strcmp(command, "help") != 0 &&
                    strcmp(command, "h")    != 0);

  if (needs_root) {
    if (lin_context_resolve_root(&ctx) != LIN_OK) {
      fprintf(stderr, "lin: %s\n", lin_error_get()->message);
      fprintf(stderr, "lin: run 'lin init' to initialize\n");
      return EXIT_FAILURE;
    }

    if (!lin_groupv2_exists(&ctx, ctx.group)) {
      fprintf(stderr, "lin: group not found: %s\n", ctx.group);
      return EXIT_FAILURE;
    }
  }

  /* dispatch */
  for (int i = 0; cmd_map[i].cmd != NULL; i++) {
    if (strcmp(command, cmd_map[i].cmd) == 0 ||
        (cmd_map[i].alias != NULL &&
         strcmp(command, cmd_map[i].alias) == 0)) {
      cmd_map[i].fn(&ctx, argc, argvi + 1, argv);
      lin_context_destroy(&ctx);
      return EXIT_SUCCESS;
    }
  }

  fprintf(stderr, "lin: unknown command: %s\n", command);
  lin_context_destroy(&ctx);
  return EXIT_FAILURE;
}
