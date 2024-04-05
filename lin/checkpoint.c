#include "env.h"
#include "checkpoint.h"
#include "linio.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

static char pathbuf[PATH_MAX_LEN];

static unsigned long lin_checkpoint_file(char *file_path) {
  unsigned long lines = lin_io_count_lines(file_path);
  if (lin_env_verbose) {
    fprintf(stdout, "%5lu: %s.\n", lines, file_path);
  }
  return lines;
}

static unsigned long lin_checkpoint_dir(char *dir_path) {
  struct stat path_st = {0};
  unsigned long total_lines = 0;
  struct dirent *dir_entry;
  DIR *dir = opendir(dir_path);
  if (dir) {
    while ((dir_entry = readdir(dir)) != NULL) {
      if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) {
        continue;
      }
      snprintf(pathbuf, sizeof(pathbuf), "%s/%s", dir_path, dir_entry->d_name);
      if (stat(pathbuf, &path_st) == 0) {
        if (S_ISREG(path_st.st_mode)) {
          total_lines += lin_checkpoint_file(pathbuf);
        } else if (S_ISDIR(path_st.st_mode)) {
          total_lines += lin_checkpoint_dir(pathbuf);
        }
      }
      pathbuf[0] = '\0'; // reset buffer
    }
    closedir(dir);
  }
  return total_lines;
}

void lin_cmd_execute_checkpoint(int argc, int argvi, char **argv) {
  struct stat path_st = {0};
  unsigned long total_lines = 0;

  while (argvi < argc) {
    if (stat(argv[argvi], &path_st) == 0) {
      if (S_ISREG(path_st.st_mode)) {
        total_lines += lin_checkpoint_file(argv[argvi]);
      } else if (S_ISDIR(path_st.st_mode)) {
        total_lines += lin_checkpoint_dir(argv[argvi]);
      }
    } else {
      fprintf(stderr, "%s: not found.\n", argv[argvi]);
    }
    argvi++;
  }
  fprintf(stdout, "-------------------------------\n");
  fprintf(stdout, "%5lu: total lines.\n", total_lines);
}

int lin_checkpoint_create(const char *path) {

}
