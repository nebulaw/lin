#ifndef LIN_STORAGE_H
#define LIN_STORAGE_H

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

/*
 * Callback for structured writes.  Receives the temp FILE*
 * and an opaque pointer supplied by the caller.
 * Must return LIN_OK (0) on success, negative on error.
 */
typedef int (*lin_write_fn)(FILE *fp, void *udata);

/*
 * Write `size` bytes from `data` atomically:
 *   1. write to <path>.XXXXXX temp file
 *   2. fsync the file
 *   3. rename over the target
 *   4. fsync the parent directory
 */
int lin_storage_write_atomic(const char *path, const void *data, size_t size);

/*
 * Same as above but the caller provides a callback that
 * writes structured data to the temp FILE*.
 */
int lin_storage_write_atomic_fn(const char *path, lin_write_fn fn, void *udata);

/*
 * Read up to `buf_size` bytes from `path` into `buf`.
 * On success, `*bytes_read` contains the number of bytes
 * actually read.
 */
int lin_storage_read_file(const char *path, void *buf, size_t buf_size,
                          size_t *bytes_read);

/*
 * Create all directories along `path` with given mode.
 * Existing directories are silently skipped.
 */
int lin_storage_ensure_dir(const char *path, mode_t mode);

/*
 * Advisory locking.  Returns a lock fd on success, -1 on error.
 * The lock is exclusive and blocks until acquired.
 */
int  lin_storage_lock(const char *path);
void lin_storage_unlock(int lock_fd);

#endif /* LIN_STORAGE_H */
