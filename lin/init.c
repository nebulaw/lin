#include "env.h"
#include "init.h"
#include "linio.h"

#include <stdio.h>
#include <sys/stat.h>

void lin_cmd_execute_init(int argc, int argvi, char **argv) {
  lin_dot_dir_initialize();
}

int lin_dot_dir_initialize(void) {
  if (lin_dot_dir_exists()) {
    return lin_dot_dir_reinitialize();
  }

  struct stat st = {0};
  if (stat(".lin", &st) == -1) {
    if (mkdir(".lin", 0755) == 0) {
      // TODO: create default group
      printf("Successfully initialized .lin directory.\n");
      return L_INIT_SUCCESS;
    } else {
      printf("Could not initialize .lin directory\n");
      return L_INIT_FAILURE;
    }
  }

  return L_INIT_SUCCESS;
}

int lin_dot_dir_reinitialize(void) {
  return 0;
}

int lin_dot_dir_exists(void) {
  return lin_io_path_exists(".lin");
}
