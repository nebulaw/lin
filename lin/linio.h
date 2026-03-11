#ifndef LIN_LINIO_H
#define LIN_LINIO_H

#include <ftw.h>
#include <limits.h>


int lin_io_simplify_path(char dst[PATH_MAX], const char *path);

int lin_io_path_exists(const char *path);

int lin_io_path_create(char *path, mode_t mode);

int lin_io_path_remove(const char *path);

int lin_io_path_remove_file_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

unsigned long lin_io_count_lines(const char *file_path);

unsigned long lin_env_get_time(void);

#endif // LIN_LINIO_H
