#include "help.h"

#include <stdio.h>
#include <string.h>

void lin_help_print(void)
{
  puts("usage: lin <command> [<args>] [options]\n");

  puts("commands:");
  puts("  init              initialize a .lin directory");
  puts("  add, a            track files in the current group");
  puts("  remove, rm        stop tracking files");
  puts("  group             manage groups (create, remove, list)");
  puts("  checkpoint, cp    snapshot current state of tracked files");
  puts("  status, st        show tracked file changes");
  puts("  log               show checkpoint history");
  puts("  stats             show project analytics");
  puts("  diff              compare two checkpoints");
  puts("  fsck              verify data integrity");
  puts("  help, h           show this help\n");

  puts("options:");
  puts("  -g, --group       specify group (default: 'default')");
  puts("  -m, --message     attach message to checkpoint");
  puts("  -v, --verbose     show detailed output");
  puts("  -V, --version     show version");
}

static void help_init(void)
{
  puts("usage: lin init\n");
  puts("create a .lin directory in the current project.");
  puts("a 'default' group is created automatically.");
}

static void help_add(void)
{
  puts("usage: lin add <file> [<file>...] [options]\n");
  puts("track files for analytics. scans each file for line");
  puts("count, size, and sha1 hash.\n");
  puts("if a file is already tracked and its hash changed,");
  puts("the manifest entry is updated.\n");
  puts("options:");
  puts("  -g, --group       add to specific group");
  puts("  -v, --verbose     show per-file details");
}

static void help_remove(void)
{
  puts("usage: lin remove <file> [<file>...] [options]\n");
  puts("stop tracking files. removes them from the group manifest.\n");
  puts("options:");
  puts("  -g, --group       remove from specific group");
  puts("  -v, --verbose     show per-file details");
}

static void help_group(void)
{
  puts("usage: lin group <subcommand> [args] [options]\n");
  puts("subcommands:");
  puts("  create, mk <name>    create a new group");
  puts("  remove, rm <name>    delete a group (cannot remove 'default')");
  puts("  list, ls             list all groups with stats\n");
  puts("options:");
  puts("  -v, --verbose        show detailed output");
}

static void help_checkpoint(void)
{
  puts("usage: lin checkpoint [options]\n");
  puts("snapshot the current state of all tracked files in the group.");
  puts("creates a binary checkpoint file with line counts, sizes,");
  puts("and hashes for every tracked file.\n");
  puts("shows delta from previous checkpoint when available.\n");
  puts("options:");
  puts("  -g, --group       checkpoint specific group");
  puts("  -m, --message     attach message to checkpoint");
  puts("  -v, --verbose     show per-file breakdown");
}

void lin_cmd_execute_help(LinContext *ctx, int argc, int argvi, char **argv)
{
  (void)ctx;

  /* skip flags */
  while (argvi < argc && argv[argvi][0] == '-')
    argvi++;

  if (argvi >= argc) {
    lin_help_print();
    return;
  }

  const char *topic = argv[argvi];

  if (strcmp(topic, "init") == 0)
    help_init();
  else if (strcmp(topic, "add") == 0)
    help_add();
  else if (strcmp(topic, "remove") == 0 || strcmp(topic, "rm") == 0)
    help_remove();
  else if (strcmp(topic, "group") == 0)
    help_group();
  else if (strcmp(topic, "checkpoint") == 0 || strcmp(topic, "cp") == 0)
    help_checkpoint();
  else
    fprintf(stderr, "lin: no help for '%s'\n", topic);
}
