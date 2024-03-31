// group command assumes that .lin/ directory exists
#define LIN_TIME_ENV
#include "env.h"
#include "group.h"

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/time.h>


void lin_cmd_execute_group(int argc, int argvi, char **argv) {
  if (argc <= 2) {
    fprintf(stdout, "lin: please specify subcommand.\n");
    fprintf(stdout, "Type 'lin help group' for more.\n");
    exit(-1);
  }

  int st;
  if (strcmp(argv[argvi], "create") == 0 || strcmp(argv[argvi], "mk") == 0) {
    if (++argvi == argc) {
      fprintf(stdout, "lin: please specify group name.\n");
      exit(EXIT_FAILURE);
    }
    st = lin_group_create(argv[argvi]);
    switch (st) {
    case L_GROUP_CREATED:
      if (lin_env_verbose) {
        fprintf(stdout, "lin: %s created.\n", argv[argvi]);
      }
    break;
    case L_GROUP_ALREADY_EXISTS:
      fprintf(stdout, "lin: %s already exists.\n", argv[argvi]);
    break;
    case L_GROUP_NOT_CREATED:
      fprintf(stderr, "lin: %s not created.\n", argv[argvi]);
      exit(EXIT_FAILURE);
    }
  } else if (strcmp(argv[argvi], "remove") == 0 || strcmp(argv[argvi], "rm") == 0) {
    // TODO: check function status
    st = lin_group_remove(argv[++argvi]);
    if (st == L_GROUP_NOT_FOUND) {
      fprintf(stdout, "lin: %s not found.\n", argv[argvi]);
      exit(EXIT_FAILURE);
    } else if (st == L_GROUP_NOT_REMOVED) {
      fprintf(stdout, "lin: %s not removed.\n", argv[argvi]);
      exit(EXIT_FAILURE);
    } else {
      if (lin_env_verbose) {
        fprintf(stdout, "lin: %s removed.\n", argv[argvi]);
      }
    }
  } else {
    fprintf(stderr, "lin: %s subcommand not found\n", argv[argvi]);
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}

int lin_group_create(const char *group_name) {
  if (lin_group_exists(group_name)) {
    return L_GROUP_ALREADY_EXISTS;
  }
  struct stat group_path_st = {0};

  char group_path[PATH_MAX_LEN];
  char objects_path[PATH_MAX_LEN];
  char info_file_path[PATH_MAX_LEN];

  snprintf(group_path, sizeof(group_path), ".lin/%s", group_name);
  snprintf(objects_path, sizeof(objects_path), ".lin/%s/objects", group_name);
  snprintf(info_file_path, sizeof(info_file_path), ".lin/%s/info", group_name);

  if (stat(group_path, &group_path_st) == -1) {
    // creating group and objects directories
    if (mkdir(group_path, 0755) != 0 || mkdir(objects_path, 0755) != 0) {
      return L_GROUP_NOT_CREATED;
    }

    // TODO: implement a function for creating info file

    // create info file for the group
    LGroupInfo group_info;
    FILE *info_file = fopen(info_file_path, "wb");
    if (info_file == NULL) {
      // cleanup (remove group path) if info file could not be created
      lin_io_path_remove(group_path);
      return L_GROUP_NOT_CREATED;
    }

    // copy group_name chars to info.group_name
    size_t group_name_len = strlen(group_name);
    strncpy(group_info.group_name, group_name, group_name_len);
    group_info.group_name[group_name_len] = '\0';
    // initial stats
    group_info.created_ms = lin_env_get_time();
    group_info.updated_ms = lin_env_get_time();
    group_info.total_checkpoints = 0;
    group_info.total_files = 0;
    group_info.total_lines = 0;

    // TODO: write info object this should decode strings
    fwrite(&group_info, sizeof(group_info), 1, info_file);
    fclose(info_file);

//    if (lin_env_verbose) {
//      LGroupInfo read_group_info;
//      info_file = fopen(info_file_path, "rb");
//      fread(&read_group_info, sizeof(read_group_info), 1, info_file);
//      fclose(info_file);
//      fprintf(stdout, "lin: info: name=%s, created=%lld, updated=%lld, total_lines=%lld, total_files=%d, total_checkpoints=%d\n",
//              read_group_info.group_name, read_group_info.created_ms, read_group_info.updated_ms,
//              read_group_info.total_lines, read_group_info.total_files, read_group_info.total_checkpoints);
//    }

    // return the status
    return L_GROUP_CREATED;
  }

  return L_GROUP_ALREADY_EXISTS;
}

int lin_group_remove(const char *group_name) {
  if (!lin_group_exists(group_name)) {
    return L_GROUP_NOT_FOUND;
  }

  char group_path[PATH_MAX_LEN];

  snprintf(group_path, sizeof(group_path), ".lin/%s", group_name);

  if (lin_io_path_remove(group_path) == 0) {
    return L_GROUP_REMOVED;
  }

  return L_GROUP_NOT_REMOVED;
}

int lin_group_exists(const char *group_name) {
  char path[PATH_MAX_LEN];
  snprintf(path, sizeof(path), ".lin/%s", group_name);
  return lin_io_path_exists(path);
}

int lin_group_info_init(const char *group_name) {
  return 0;
}

int lin_group_info_update(const char *group_name, LGroupInfo info) {
  return 0;
}
