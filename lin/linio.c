#include "linio.h"
#include "env.h"

#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


#define LIN_FILE_BUFFER_SIZE 8192

// Simplify a path by removing redundant slashes and dot directories
int lin_io_simplify_path(char dst[PATH_MAX], char *path)
{
  return realpath(path, dst) == NULL ? 1 : 0;
}

int lin_io_path_exists(char *path)
{
  struct stat st = {0};
  return stat(path, &st) == 0;
}

int lin_io_path_create(char *path, mode_t mode)
{
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

int lin_io_path_remove(char *path)
{
  return nftw(path, lin_io_path_remove_file_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int lin_io_path_remove_file_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
  return remove(fpath);
}

unsigned long lin_io_count_lines(const char *file_path)
{
  char buf[LIN_FILE_BUFFER_SIZE];
  FILE *file;
  size_t nfound = 0;
  int nread;

  file = fopen(file_path, "r");
  if (file == NULL) {
    return 0;
  }

  while ((nread = (int) fread(buf, 1, LIN_FILE_BUFFER_SIZE, file)) > 0) {
    char const *p;
    for (p = buf; (p = memchr(p, '\n', buf + nread - p)); nfound++, p++);
  }

  if (ferror(file)) {
    perror("Error reading file");
    fclose(file);
    return 1;
  }

  fclose(file);

  return nfound;
}

unsigned long lin_env_get_time(void)
{
  struct timeval timeval;
  gettimeofday(&timeval, NULL);
  return (unsigned long) (timeval.tv_sec) * 1000 + (unsigned long) (timeval.tv_usec) / 1000;
}
