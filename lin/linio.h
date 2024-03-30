#ifndef LIN_LINIO_H
#define LIN_LINIO_H

#define _XOPEN_SOURCE 500
#include <ftw.h>

#define PATH_MAX_LEN    200

int lin_io_path_exists(char *path);
int lin_io_path_create(char *path, mode_t mode);
int lin_io_path_remove(char *path);
int lin_io_path_remove_file_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
unsigned long lin_io_count_lines(const char *file_path);
unsigned long lin_env_get_time(void);

#endif //LIN_LINIO_H
