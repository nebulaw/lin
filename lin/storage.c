#include "storage.h"
#include "error.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/*  internal helpers                                                   */
/* ------------------------------------------------------------------ */

static int sync_parent_dir(const char *file_path)
{
  char dir[PATH_MAX];
  snprintf(dir, sizeof(dir), "%s", file_path);

  char *sep = strrchr(dir, '/');
  if (sep == NULL) {
    dir[0] = '.';
    dir[1] = '\0';
  } else if (sep == dir) {
    dir[1] = '\0';
  } else {
    *sep = '\0';
  }

  int fd = open(dir, O_RDONLY);
  if (fd < 0)
    return LIN_ERR_IO_FSYNC;

  fsync(fd);
  close(fd);
  return LIN_OK;
}

/*
 * Open a temp file in the same directory as `target`.
 * Fills `tmp_path` with the resulting path and returns
 * the file descriptor, or -1 on error.
 */
static int open_temp_beside(const char *target, char *tmp_path, size_t tmp_size)
{
  int n = snprintf(tmp_path, tmp_size, "%s.XXXXXX", target);
  if (n < 0 || (size_t)n >= tmp_size) {
    lin_error_set(LIN_ERR_PATH_TOO_LONG, "temp path for %s", target);
    return -1;
  }

  int fd = mkstemp(tmp_path);
  if (fd < 0) {
    lin_error_set(LIN_ERR_IO_OPEN, "mkstemp %s: %s", tmp_path, strerror(errno));
    return -1;
  }

  /* prevent fd from leaking to child processes */
  fcntl(fd, F_SETFD, FD_CLOEXEC);
  return fd;
}

/* ------------------------------------------------------------------ */
/*  public API                                                         */
/* ------------------------------------------------------------------ */

int lin_storage_write_atomic(const char *path, const void *data, size_t size)
{
  char tmp[PATH_MAX];
  int fd = open_temp_beside(path, tmp, sizeof(tmp));
  if (fd < 0)
    return LIN_ERR_IO_OPEN;

  const unsigned char *p = data;
  size_t remaining = size;

  while (remaining > 0) {
    ssize_t nw = write(fd, p, remaining);
    if (nw < 0) {
      if (errno == EINTR)
        continue;
      lin_error_set(LIN_ERR_IO_WRITE, "write %s: %s", tmp, strerror(errno));
      close(fd);
      unlink(tmp);
      return LIN_ERR_IO_WRITE;
    }
    p += nw;
    remaining -= (size_t)nw;
  }

  if (fsync(fd) != 0) {
    lin_error_set(LIN_ERR_IO_FSYNC, "fsync %s: %s", tmp, strerror(errno));
    close(fd);
    unlink(tmp);
    return LIN_ERR_IO_FSYNC;
  }
  close(fd);

  if (rename(tmp, path) != 0) {
    lin_error_set(LIN_ERR_IO_RENAME, "rename %s -> %s: %s",
                  tmp, path, strerror(errno));
    unlink(tmp);
    return LIN_ERR_IO_RENAME;
  }

  sync_parent_dir(path);
  return LIN_OK;
}

int lin_storage_write_atomic_fn(const char *path, lin_write_fn fn, void *udata)
{
  char tmp[PATH_MAX];
  int fd = open_temp_beside(path, tmp, sizeof(tmp));
  if (fd < 0)
    return LIN_ERR_IO_OPEN;

  FILE *fp = fdopen(fd, "wb");
  if (fp == NULL) {
    lin_error_set(LIN_ERR_IO_OPEN, "fdopen %s: %s", tmp, strerror(errno));
    close(fd);
    unlink(tmp);
    return LIN_ERR_IO_OPEN;
  }

  int rc = fn(fp, udata);
  if (rc != LIN_OK) {
    fclose(fp);
    unlink(tmp);
    return rc;
  }

  fflush(fp);
  if (fsync(fileno(fp)) != 0) {
    lin_error_set(LIN_ERR_IO_FSYNC, "fsync %s: %s", tmp, strerror(errno));
    fclose(fp);
    unlink(tmp);
    return LIN_ERR_IO_FSYNC;
  }
  fclose(fp);

  if (rename(tmp, path) != 0) {
    lin_error_set(LIN_ERR_IO_RENAME, "rename %s -> %s: %s",
                  tmp, path, strerror(errno));
    unlink(tmp);
    return LIN_ERR_IO_RENAME;
  }

  sync_parent_dir(path);
  return LIN_OK;
}

int lin_storage_read_file(const char *path, void *buf, size_t buf_size,
                          size_t *bytes_read)
{
  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    lin_error_set(LIN_ERR_IO_OPEN, "open %s: %s", path, strerror(errno));
    return LIN_ERR_IO_OPEN;
  }

  size_t n = fread(buf, 1, buf_size, fp);
  if (ferror(fp)) {
    lin_error_set(LIN_ERR_IO_READ, "read %s: %s", path, strerror(errno));
    fclose(fp);
    return LIN_ERR_IO_READ;
  }

  fclose(fp);

  if (bytes_read != NULL)
    *bytes_read = n;

  return LIN_OK;
}

int lin_storage_ensure_dir(const char *path, mode_t mode)
{
  char tmp[PATH_MAX];
  struct stat st;

  int n = snprintf(tmp, sizeof(tmp), "%s", path);
  if (n < 0 || (size_t)n >= sizeof(tmp)) {
    lin_error_set(LIN_ERR_PATH_TOO_LONG, "%s", path);
    return LIN_ERR_PATH_TOO_LONG;
  }

  /* strip trailing slash */
  size_t len = (size_t)n;
  if (len > 1 && tmp[len - 1] == '/')
    tmp[--len] = '\0';

  /* already exists */
  if (stat(tmp, &st) == 0)
    return S_ISDIR(st.st_mode) ? LIN_OK : LIN_ERR_PATH_INVALID;

  /* create each component from left to right */
  for (char *p = tmp + 1; *p; p++) {
    if (*p != '/')
      continue;

    *p = '\0';
    if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
      lin_error_set(LIN_ERR_IO_MKDIR, "mkdir %s: %s", tmp, strerror(errno));
      *p = '/';
      return LIN_ERR_IO_MKDIR;
    }
    *p = '/';
  }

  if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
    lin_error_set(LIN_ERR_IO_MKDIR, "mkdir %s: %s", tmp, strerror(errno));
    return LIN_ERR_IO_MKDIR;
  }

  return LIN_OK;
}

int lin_storage_lock(const char *path)
{
  char lock_path[PATH_MAX];
  int n = snprintf(lock_path, sizeof(lock_path), "%s.lock", path);
  if (n < 0 || (size_t)n >= sizeof(lock_path)) {
    lin_error_set(LIN_ERR_PATH_TOO_LONG, "lock path for %s", path);
    return -1;
  }

  int fd = open(lock_path, O_RDWR | O_CREAT | O_CLOEXEC, 0644);
  if (fd < 0) {
    lin_error_set(LIN_ERR_IO_LOCK, "open lock %s: %s",
                  lock_path, strerror(errno));
    return -1;
  }

  if (flock(fd, LOCK_EX) != 0) {
    lin_error_set(LIN_ERR_IO_LOCK, "flock %s: %s",
                  lock_path, strerror(errno));
    close(fd);
    return -1;
  }

  return fd;
}

void lin_storage_unlock(int lock_fd)
{
  if (lock_fd >= 0) {
    flock(lock_fd, LOCK_UN);
    close(lock_fd);
  }
}
