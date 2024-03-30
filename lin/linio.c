#include "env.h"
#include "linio.h"

#include <ftw.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#define LIN_FILE_BUFFER_SIZE 4096

int lin_io_path_exists(char *path) {
  struct stat st = {0};
  return stat(path, &st) == 0;
}

int lin_io_path_create(char *path, mode_t mode) {
  for (char *p = strchr(path + 1, '/'); p; p = strchr(p + 1, '/')) {
    *p = '\0';
    if (mkdir(path, mode) == -1) {
      if (errno != EEXIST) {
        *p = '/';
        return L_PATH_NOT_CREATED;
      }
    }
    *p = '/';
  }
  return L_PATH_CREATED;
}

int lin_io_path_remove(char *path) {
  return nftw(path, lin_io_path_remove_file_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int lin_io_path_remove_file_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
  return remove(fpath);
}

unsigned long lin_io_count_lines(const char *file_path) {
  FILE *fptr = fopen(file_path, "r");
  if (fptr == NULL) {
    return -1;
  }

  int line_count = 0;
  char buffer[LIN_FILE_BUFFER_SIZE];
  size_t bytes_read;

  int previous_char_was_newline = 1; // Initialize to 1 to handle files starting with a newline

  while ((bytes_read = fread(buffer, 1, LIN_FILE_BUFFER_SIZE, fptr)) > 0) {
    for (size_t i = 0; i < bytes_read; i++) {
      if (buffer[i] == '\n') {
        line_count++;
        previous_char_was_newline = 1;
      } else {
        previous_char_was_newline = 0;
      }
    }
  }

  // If the last character in the fptr is not a newline, increment line count
  if (!previous_char_was_newline) {
    line_count++;
  }

  free(fptr);

  return line_count;
}

unsigned long lin_env_get_time(void) {
  struct timeval timeval;
  gettimeofday(&timeval, NULL);
  return (unsigned long) (timeval.tv_sec) * 1000 +
         (unsigned long) (timeval.tv_usec) / 1000;
}
