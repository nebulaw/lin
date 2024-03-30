#include "help.h"

void lin_help() {
  puts("Usage: lin [command] [subcommand] [args] [options]\n");

  puts("Command:");
  puts("  init      initialize a lin directory.");
  puts("  add       add files to the specified group");
  puts("  group     manage several groups\n");

  puts("Type 'help [command]' to list available subcommands.");
}

void lin_help_group() {
  puts("Usage: lin group [subcommand] [options]\n");

  puts("Examples:");
  puts("  lin group create [group-name]");
  puts("  lin group remove [group-name]\n");

  puts("Subcommand:");
  puts("  create    create group by specifying name");
  puts("  remove    delete group\n");

  puts("  -g, --group       specify group to work with");
  puts("  -v, --verbose     be verbose on your actions");
}

void lin_help_add() {
  puts("Usage: lin add <file> [<file>...] [options]\n");

  puts("Options");
  puts("  -g, --group       specify group to add file to");
  puts("  -v, --verbose     be verbose on your actions");
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
  } else {
    lin_help();
  }
}
