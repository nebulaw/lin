#define _LIN_ENV_INIT
#include "env.h"
#include "help.h"
#include "init.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

typedef void (*lin_command_execute)(int argc, int argvi, char **argv);

struct LCmdProcessMap {
  const char *cmd;
  lin_command_execute process;
};

static struct LCmdProcessMap cmd_map[] = {
    { "help",   lin_cmd_execute_help },
    { "init",   lin_cmd_execute_init },
    { NULL,     NULL},
};

static struct option arg_options[] = {
    { "group",    required_argument,  0, 'g' },
    { "message",  required_argument,  0, 'm' },
    { "verbose",  no_argument,        0, 'v' },
    { 0, 0, 0, 0},
};

int main(int argc, char **argv) {
  char **argv_ptr = argv;
  char *command;
  int is_group_present = 0;
  int is_message_present = 0;
  int is_processed = 0;

  if (argc < 2) {
    return EXIT_FAILURE;
  }

  // obtain command while skipping first flags
  int argvi = 1;
  while (argv[argvi] && *argv[argvi] == '-') {
    argvi++;
  }
  command = argv[argvi];

  // parsing flags rearranges program arguments so that the flags are placed at
  int op;
  while ((op = getopt_long(argc, argv_ptr, "g:m:v", arg_options, NULL) != -1)) {
    switch (op) {
    case 'g':
      lin_env_set_group(optarg, strlen(optarg));
      is_message_present = 1;
      break;
    case 'v':
      lin_env_set_verbose(1);
      break;
    case 'm':
      lin_env_set_message(optarg, strlen(optarg));
      break;
    case '?':
      exit(EXIT_FAILURE);
    default:
      break;
    }
  }

  if (strcmp(command, "init") != 0) {
    // TODO: check if .lin directory exists
  }

  // TODO: check if group exists

  for (int i = 0; cmd_map[i].cmd != NULL; i++) {
    if (strcmp(command, cmd_map[i].cmd) == 0) {
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
