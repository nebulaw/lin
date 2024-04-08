#define _LIN_ENV_INIT
#include "env.h"
#include "add.h"
#include "group.h"
#include "help.h"
#include "init.h"
#include "checkpoint.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

typedef void (*lin_command_execute)(int argc, int argvi, char **argv);

struct LCmdProcessMap {
  const char *cmd;
  const char *alias;
  lin_command_execute process;
};

static struct LCmdProcessMap cmd_map[] = {
    { .cmd = "help",        .alias = "h",   lin_cmd_execute_help },
    { .cmd = "init",        .alias = "-",   lin_cmd_execute_init },
    { .cmd = "group",       .alias = "-",   lin_cmd_execute_group },
    { .cmd = "checkpoint",  .alias = "cp",  lin_cmd_execute_checkpoint },
    { .cmd = "add",         .alias = "a",   lin_cmd_execute_add },
    { NULL, NULL},
};

static struct option arg_options[] = {
    { "group",    required_argument,  0,   'g' },
    { "message",  required_argument,  0,   'm' },
    { "verbose",  no_argument,        0,   'v' },
    { 0, 0, 0, 0},
};

int main(int argc, char **argv) {
  char **argv_ptr = argv;
  char *command;
  int is_group_present = 0;
  int is_message_present = 0;
  int is_processed = 0;

  if (argc < 2) {
    fprintf(stdout, "lin: no command specified\n");
    lin_help();
    exit(EXIT_FAILURE);
  }

  // obtain command while skipping first flags
  int argvi = 1;
  while (argvi < argc && *argv[argvi] == '-') {
    argvi++;
  }
  command = argv[argvi];

  // parsing flags rearranges program arguments so that the flags are placed at
  int op;
  while ((op = getopt_long(argc, argv_ptr, "m:g:v", arg_options, NULL)) != -1) {
    switch (op) {
    case 'g': lin_env_set_group(optarg, strlen(optarg)); is_group_present = 1; break;
    case 'v': lin_env_set_verbose(1); break;
    case 'm': lin_env_set_message(optarg, strlen(optarg)); is_message_present = 1; break;
    case '?': exit(EXIT_FAILURE);
    default: break;
    }
  }

  // as flags are in the beginning of the argv
  // we traverse until the command is not found
  argvi = 0;
  while (argvi < argc && strcmp(argv[argvi], command) != 0)
    argvi++;

  // preprocess before command will be executed
  // if the command is not 'init', then .lin
  // directory should exist
  if (strcmp(command, "init") != 0 && strcmp(command, "help") != 0) {
    // TODO: check if .lin directory exists
    if (!lin_dot_dir_exists()) {
      fprintf(stderr, "lin: .lin directory not found\n");
      exit(EXIT_FAILURE);
    } else if (!is_group_present) {
      lin_env_set_group("default", 7);
    } else if (!lin_group_exists(lin_env_group)) {
      fprintf(stderr, "lin: group not found: %s\n", lin_env_group);
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; cmd_map[i].cmd != NULL; i++) {
    if (strcmp(command, cmd_map[i].cmd) == 0 ||
        (*cmd_map[i].alias != '-' && strcmp(command, cmd_map[i].alias) == 0)) {
      cmd_map[i].process(argc, argvi + 1, argv);
      is_processed = 1;
      break;
    }
  }

  if (!is_processed) {
    fprintf(stderr, "lin: command not found: %s\n", command);
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
