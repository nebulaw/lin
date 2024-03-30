#ifndef _LIN_SHARED_ENV
#define _LIN_SHARED_ENV

// this is for traversing directory tree
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#ifndef _LIN_ENV_INIT
#define _LIN_EXTERN extern
#else
#define _LIN_EXTERN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

_LIN_EXTERN char lin_env_group[25];
_LIN_EXTERN char lin_env_message[201];
_LIN_EXTERN int lin_env_verbose;

enum LStatus {
  L_FAILURE = -1,
  L_SUCCESS,

  L_INIT_SUCCESS,
  L_INIT_FAILURE,

  L_PATH_CREATED,
  L_PATH_NOT_CREATED,

  L_GROUP_CREATED,
  L_GROUP_ALREADY_EXISTS,
  L_GROUP_NOT_CREATED,
  L_GROUP_REMOVED,
  L_GROUP_NOT_REMOVED,
  L_GROUP_NOT_FOUND,
};

void lin_env_set_message(char *message, size_t len);
void lin_env_set_verbose(int verbose);
void lin_env_set_group(char *group_name, size_t len);

#endif // _LIN_SHARED_ENV
