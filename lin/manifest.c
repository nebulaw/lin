#include "manifest.h"
#include "error.h"
#include "storage.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  serialization helpers                                              */
/* ------------------------------------------------------------------ */

static int parse_entry(const char *line, LinManifestEntry *entry)
{
  /* format: <sha1>\t<line_count>\t<file_size>\t<added_ms>\t<path>\n */
  char sha1[LIN_SHA1_HEX_LEN + 1];
  uint64_t lc, fs, ts;
  char path[PATH_MAX];

  char fmt[64];
  snprintf(fmt, sizeof(fmt),
           "%%40s\t%%llu\t%%llu\t%%llu\t%%%d[^\n]", PATH_MAX - 1);

  int n = sscanf(line, fmt,
                 sha1, &lc, &fs, &ts, path);

  if (n != 5) {
    lin_error_set(LIN_ERR_MANIFEST_PARSE,
                  "malformed manifest line: expected 5 fields, got %d", n);
    return LIN_ERR_MANIFEST_PARSE;
  }

  memcpy(entry->sha1, sha1, LIN_SHA1_HEX_LEN + 1);
  entry->line_count = lc;
  entry->file_size  = fs;
  entry->added_ms   = ts;
  snprintf(entry->path, sizeof(entry->path), "%s", path);

  return LIN_OK;
}

typedef struct {
  const LinManifestEntry *entries;
  int count;
} ManifestWriteCtx;

static int write_manifest_cb(FILE *fp, void *udata)
{
  ManifestWriteCtx *wctx = udata;

  if (fprintf(fp, "%s\n", LIN_MANIFEST_HEADER) < 0) {
    lin_error_set(LIN_ERR_IO_WRITE, "failed to write manifest header");
    return LIN_ERR_IO_WRITE;
  }

  for (int i = 0; i < wctx->count; i++) {
    const LinManifestEntry *e = &wctx->entries[i];
    if (fprintf(fp, "%s\t%llu\t%llu\t%llu\t%s\n",
                e->sha1,
                (unsigned long long)e->line_count,
                (unsigned long long)e->file_size,
                (unsigned long long)e->added_ms,
                e->path) < 0) {
      lin_error_set(LIN_ERR_IO_WRITE, "failed to write manifest entry: %s",
                    e->path);
      return LIN_ERR_IO_WRITE;
    }
  }

  return LIN_OK;
}

/* ------------------------------------------------------------------ */
/*  path construction                                                  */
/* ------------------------------------------------------------------ */

static int manifest_path(const LinContext *ctx, const char *group,
                         char *out, size_t size)
{
  return lin_context_path(ctx, out, size, "/%s/" LIN_MANIFEST_FILE, group);
}

/* ------------------------------------------------------------------ */
/*  public API                                                         */
/* ------------------------------------------------------------------ */

int lin_manifest_load(const LinContext *ctx, const char *group,
                      LinManifestEntry *entries, int max_entries)
{
  char path[PATH_MAX];
  int rc = manifest_path(ctx, group, path, sizeof(path));
  if (rc != LIN_OK)
    return rc;

  FILE *fp = fopen(path, "r");
  if (fp == NULL) {
    if (errno == ENOENT) {
      /* no manifest yet is valid: zero entries */
      return 0;
    }
    lin_error_set(LIN_ERR_IO_OPEN, "open %s: %s", path, strerror(errno));
    return LIN_ERR_IO_OPEN;
  }

  char line[PATH_MAX + 256];
  int count = 0;

  /* validate header */
  if (fgets(line, sizeof(line), fp) == NULL) {
    fclose(fp);
    return 0; /* empty file */
  }

  /* strip newline for comparison */
  size_t hlen = strlen(line);
  if (hlen > 0 && line[hlen - 1] == '\n')
    line[hlen - 1] = '\0';

  if (strcmp(line, LIN_MANIFEST_HEADER) != 0) {
    lin_error_set(LIN_ERR_MANIFEST_PARSE,
                  "invalid manifest header: '%s'", line);
    fclose(fp);
    return LIN_ERR_MANIFEST_PARSE;
  }

  /* parse entries */
  while (fgets(line, sizeof(line), fp) != NULL) {
    if (line[0] == '\n' || line[0] == '#')
      continue;

    if (count >= max_entries) {
      lin_error_set(LIN_ERR_MANIFEST_FULL,
                    "manifest exceeds %d entries", max_entries);
      fclose(fp);
      return LIN_ERR_MANIFEST_FULL;
    }

    rc = parse_entry(line, &entries[count]);
    if (rc != LIN_OK) {
      fclose(fp);
      return rc;
    }
    count++;
  }

  fclose(fp);
  return count;
}

int lin_manifest_save(const LinContext *ctx, const char *group,
                      const LinManifestEntry *entries, int count)
{
  char path[PATH_MAX];
  int rc = manifest_path(ctx, group, path, sizeof(path));
  if (rc != LIN_OK)
    return rc;

  ManifestWriteCtx wctx = { .entries = entries, .count = count };
  return lin_storage_write_atomic_fn(path, write_manifest_cb, &wctx);
}

int lin_manifest_append(const LinContext *ctx, const char *group,
                        const LinManifestEntry *entry)
{
  LinManifestEntry *buf = malloc(sizeof(LinManifestEntry) * LIN_MANIFEST_MAX);
  if (buf == NULL) {
    lin_error_set(LIN_ERR_NOMEM, "manifest buffer allocation");
    return LIN_ERR_NOMEM;
  }

  int count = lin_manifest_load(ctx, group, buf, LIN_MANIFEST_MAX);
  if (count < 0) {
    free(buf);
    return count;
  }

  /* check for duplicate path */
  for (int i = 0; i < count; i++) {
    if (strcmp(buf[i].path, entry->path) == 0) {
      free(buf);
      lin_error_set(LIN_ERR_MANIFEST_ENTRY_EXISTS,
                    "already tracked: %s", entry->path);
      return LIN_ERR_MANIFEST_ENTRY_EXISTS;
    }
  }

  if (count >= LIN_MANIFEST_MAX) {
    free(buf);
    lin_error_set(LIN_ERR_MANIFEST_FULL,
                  "manifest capacity reached (%d)", LIN_MANIFEST_MAX);
    return LIN_ERR_MANIFEST_FULL;
  }

  buf[count] = *entry;
  int rc = lin_manifest_save(ctx, group, buf, count + 1);
  free(buf);
  return rc;
}

int lin_manifest_remove(const LinContext *ctx, const char *group,
                        const char *file_path)
{
  LinManifestEntry *buf = malloc(sizeof(LinManifestEntry) * LIN_MANIFEST_MAX);
  if (buf == NULL) {
    lin_error_set(LIN_ERR_NOMEM, "manifest buffer allocation");
    return LIN_ERR_NOMEM;
  }

  int count = lin_manifest_load(ctx, group, buf, LIN_MANIFEST_MAX);
  if (count < 0) {
    free(buf);
    return count;
  }

  int found = -1;
  for (int i = 0; i < count; i++) {
    if (strcmp(buf[i].path, file_path) == 0) {
      found = i;
      break;
    }
  }

  if (found < 0) {
    free(buf);
    lin_error_set(LIN_ERR_MANIFEST_ENTRY_NOT_FOUND,
                  "not tracked: %s", file_path);
    return LIN_ERR_MANIFEST_ENTRY_NOT_FOUND;
  }

  /* shift entries down */
  for (int i = found; i < count - 1; i++)
    buf[i] = buf[i + 1];

  int rc = lin_manifest_save(ctx, group, buf, count - 1);
  free(buf);
  return rc;
}

int lin_manifest_find(const LinContext *ctx, const char *group,
                      const char *file_path, LinManifestEntry *out)
{
  LinManifestEntry *buf = malloc(sizeof(LinManifestEntry) * LIN_MANIFEST_MAX);
  if (buf == NULL) {
    lin_error_set(LIN_ERR_NOMEM, "manifest buffer allocation");
    return LIN_ERR_NOMEM;
  }

  int count = lin_manifest_load(ctx, group, buf, LIN_MANIFEST_MAX);
  if (count < 0) {
    free(buf);
    return count;
  }

  for (int i = 0; i < count; i++) {
    if (strcmp(buf[i].path, file_path) == 0) {
      if (out != NULL)
        *out = buf[i];
      free(buf);
      return i;
    }
  }

  free(buf);
  lin_error_set(LIN_ERR_MANIFEST_ENTRY_NOT_FOUND,
                "not tracked: %s", file_path);
  return LIN_ERR_MANIFEST_ENTRY_NOT_FOUND;
}

int lin_manifest_update(const LinContext *ctx, const char *group,
                        const LinManifestEntry *entry)
{
  LinManifestEntry *buf = malloc(sizeof(LinManifestEntry) * LIN_MANIFEST_MAX);
  if (buf == NULL) {
    lin_error_set(LIN_ERR_NOMEM, "manifest buffer allocation");
    return LIN_ERR_NOMEM;
  }

  int count = lin_manifest_load(ctx, group, buf, LIN_MANIFEST_MAX);
  if (count < 0) {
    free(buf);
    return count;
  }

  int found = -1;
  for (int i = 0; i < count; i++) {
    if (strcmp(buf[i].path, entry->path) == 0) {
      found = i;
      break;
    }
  }

  if (found < 0) {
    free(buf);
    lin_error_set(LIN_ERR_MANIFEST_ENTRY_NOT_FOUND,
                  "not tracked: %s", entry->path);
    return LIN_ERR_MANIFEST_ENTRY_NOT_FOUND;
  }

  buf[found] = *entry;
  int rc = lin_manifest_save(ctx, group, buf, count);
  free(buf);
  return rc;
}
