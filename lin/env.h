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


#define GROUP_MAX_LEN 24
#define MESSAGE_MAX_LEN 200

_LIN_EXTERN char lin_env_group[GROUP_MAX_LEN + 1];
_LIN_EXTERN char lin_env_message[MESSAGE_MAX_LEN + 1];
_LIN_EXTERN int lin_env_verbose;

enum LStatus
{
    L_SUCCESS,
    L_FAILURE,

    L_INIT_SUCCESS,
    L_INIT_FAILURE,

    L_PATH_CREATED,
    L_PATH_NOT_CREATED,
    L_FILE_NOT_FOUND,

    L_GROUP_CREATED,
    L_GROUP_ALREADY_EXISTS,
    L_GROUP_NOT_CREATED,
    L_GROUP_REMOVED,
    L_GROUP_NOT_REMOVED,
    L_GROUP_NOT_FOUND,
    L_GROUP_INFO_NOT_FOUND,

    L_ADD_SUCCESS,
    L_ADD_FAILURE,

    L_OBJECT_CREATED,
    L_OBJECT_NOT_CREATED,
    L_OBJECT_REMOVED,
    L_OBJECT_NOT_REMOVED,
    L_OBJECT_NOT_FOUND,
    L_OBJECT_NAME_TOO_SHORT,
};

void lin_env_set_message(char *message, size_t len);

void lin_env_set_verbose(int verbose);

void lin_env_set_group(char *group_name, size_t len);

#endif // _LIN_SHARED_ENV
