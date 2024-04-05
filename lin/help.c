#include "help.h"

void lin_help() {
  puts("Usage: lin [command] [subcommand] [args] [options]\n");

  puts("Command:");
  puts("  init            Initialize a lin directory.");
  puts("  add             Add files to the specified group.");
  puts("  group           Manage several groups.");
  puts("  checkpoint, cp  Checkpoint current workspace.");
  puts("  help, h         Render help menu.\n");

  puts("Type 'help [command]' to list available subcommands.");
}

static void lin_help_group() {
  puts("Usage: lin group [subcommand] [options]\n");

  puts("Examples:");
  puts("  lin group create [group-name]");
  puts("  lin group remove [group-name]\n");

  puts("Subcommand:");
  puts("  create, mk    Add a new group.");
  puts("  remove, rm    Delete an existing group.\n");

  // puts("  -g, --group       specify group to work with");
  puts("  -v, --verbose     Be verbose on your actions.");
}

static void lin_help_add() {
  puts("Usage: lin add <file> [<file>...] [options]\n");

  puts("Options");
  puts("  -g, --group       Specify group to add file to.");
  puts("  -v, --verbose     Be verbose on your actions.");
}

static void lin_help_checkpoint() {
  puts("Usage: lin checkpoint <group>\n");

  puts("Options:");
  puts("  -v, --verbose     Be verbose of your actions.");
}

void lin_cmd_execute_help(int argc, int argvi, char **argv) {
  while (argvi < argc && *argv[argvi] == '-')
    argvi++;

  if (argvi == argc) {
    lin_help();
    return;
  }

  if (strcmp(argv[argvi], "group") == 0) {
    lin_help_group();
  } else if (strcmp(argv[argvi], "add") == 0) {
    lin_help_add();
  } else if (strcmp(argv[argvi], "cp") == 0 ||
             strcmp(argv[argvi], "checkpoint") == 0) {
    lin_help_checkpoint();
  } else {
    fprintf(stderr, "lin: %s subcommand not found\n", argv[argvi]);
  }
}
