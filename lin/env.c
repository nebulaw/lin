#include "env.h"

void lin_env_set_message(char *message, size_t len) {
  if (len > 200) {
    return;
  }
  if (message) {
    strncpy(lin_env_message, message, len);
    lin_env_message[len] = '\0';
  }
}

void lin_env_set_verbose(int verbose) {
  if (verbose == 0 || verbose == 1) {
    lin_env_verbose = verbose;
  }
}

void lin_env_set_group(char *group_name, size_t len) {
  if (len > 24) {
    return;
  }
  if (group_name) {
    strncpy(lin_env_group, group_name, len);
    lin_env_group[len] = '\0';
  }
}