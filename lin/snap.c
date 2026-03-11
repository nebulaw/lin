#include "snap.h"
#include "analysis.h"
#include "error.h"
#include "format.h"
#include "storage.h"
#include "linio.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  path helpers                                                       */
/* ------------------------------------------------------------------ */

static int snap_dir_path(const LinContext *ctx, const char *group,
                         char *out, size_t size)
{
  return lin_context_path(ctx, out, size,
                          "/%s/" LIN_SNAP_DIR, group);
}

static int snap_file_path(const LinContext *ctx, const char *group,
                          uint32_t id, char *out, size_t size)
{
  return lin_context_path(ctx, out, size,
                          "/%s/" LIN_SNAP_DIR "/%0*u" LIN_SNAP_EXT,
                          group, LIN_SNAP_ID_WIDTH, id);
}

/* ------------------------------------------------------------------ */
/*  binary write callback                                              */
/* ------------------------------------------------------------------ */

typedef struct {
  const LinSnapshot *snap;
} SnapWriteCtx;

static int write_snap_cb(FILE *fp, void *udata)
{
  SnapWriteCtx *wctx = udata;
  const LinSnapshot *s = wctx->snap;

  /* file header */
  int rc = lin_format_write_header(fp, LIN_MAGIC_CKPT, LIN_FORMAT_V1);
  if (rc != LIN_OK)
    return rc;

  /* checkpoint header fields */
  if (fwrite(&s->id,           sizeof(s->id),           1, fp) != 1 ||
      fwrite(&s->timestamp_ms, sizeof(s->timestamp_ms), 1, fp) != 1 ||
      fwrite(&s->total_lines,  sizeof(s->total_lines),  1, fp) != 1 ||
      fwrite(&s->total_size,   sizeof(s->total_size),   1, fp) != 1 ||
      fwrite(&s->total_files,  sizeof(s->total_files),  1, fp) != 1 ||
      fwrite(&s->message_len,  sizeof(s->message_len),  1, fp) != 1) {
    lin_error_set(LIN_ERR_IO_WRITE, "failed to write checkpoint header");
    return LIN_ERR_IO_WRITE;
  }

  /* message (variable length) */
  if (s->message_len > 0) {
    if (fwrite(s->message, 1, s->message_len, fp) != s->message_len) {
      lin_error_set(LIN_ERR_IO_WRITE, "failed to write checkpoint message");
      return LIN_ERR_IO_WRITE;
    }
  }

  /* entry count */
  if (fwrite(&s->entry_count, sizeof(s->entry_count), 1, fp) != 1) {
    lin_error_set(LIN_ERR_IO_WRITE, "failed to write entry count");
    return LIN_ERR_IO_WRITE;
  }

  /* per-file entries */
  for (uint32_t i = 0; i < s->entry_count; i++) {
    const LinSnapEntry *e = &s->entries[i];

    if (fwrite(e->sha1,       LIN_SHA1_HEX_LEN, 1, fp) != 1 ||
        fwrite(&e->line_count, sizeof(e->line_count), 1, fp) != 1 ||
        fwrite(&e->file_size,  sizeof(e->file_size),  1, fp) != 1 ||
        fwrite(&e->path_len,   sizeof(e->path_len),   1, fp) != 1) {
      lin_error_set(LIN_ERR_IO_WRITE, "failed to write entry %u", i);
      return LIN_ERR_IO_WRITE;
    }

    if (e->path_len > 0) {
      if (fwrite(e->path, 1, e->path_len, fp) != e->path_len) {
        lin_error_set(LIN_ERR_IO_WRITE, "failed to write entry path %u", i);
        return LIN_ERR_IO_WRITE;
      }
    }
  }

  return LIN_OK;
}

/* ------------------------------------------------------------------ */
/*  public API                                                         */
/* ------------------------------------------------------------------ */

int lin_snap_create(const LinContext *ctx, const char *group,
                    const char *message)
{
  /* ensure checkpoints directory exists */
  char dir[PATH_MAX];
  int rc = snap_dir_path(ctx, group, dir, sizeof(dir));
  if (rc != LIN_OK)
    return rc;

  rc = lin_storage_ensure_dir(dir, 0755);
  if (rc != LIN_OK)
    return rc;

  /* load tracked manifest */
  LinManifestEntry *manifest = malloc(sizeof(LinManifestEntry) * LIN_MANIFEST_MAX);
  if (manifest == NULL) {
    lin_error_set(LIN_ERR_NOMEM, "manifest allocation");
    return LIN_ERR_NOMEM;
  }

  int file_count = lin_manifest_load(ctx, group, manifest, LIN_MANIFEST_MAX);
  if (file_count < 0) {
    free(manifest);
    return file_count;
  }

  /* scan all tracked files */
  LinSnapEntry *entries = NULL;
  if (file_count > 0) {
    entries = calloc((size_t)file_count, sizeof(LinSnapEntry));
    if (entries == NULL) {
      free(manifest);
      lin_error_set(LIN_ERR_NOMEM, "snap entries allocation");
      return LIN_ERR_NOMEM;
    }
  }

  uint64_t total_lines = 0;
  uint64_t total_size  = 0;

  for (int i = 0; i < file_count; i++) {
    LinFileStats fs;
    rc = lin_analysis_scan_file(manifest[i].path, &fs);

    memcpy(entries[i].sha1,
           rc == LIN_OK ? fs.sha1 : manifest[i].sha1,
           LIN_SHA1_HEX_LEN + 1);

    entries[i].line_count = rc == LIN_OK ? fs.line_count : 0;
    entries[i].file_size  = rc == LIN_OK ? fs.file_size  : 0;
    entries[i].path_len   = (uint16_t)strlen(manifest[i].path);
    snprintf(entries[i].path, sizeof(entries[i].path), "%s", manifest[i].path);

    if (rc == LIN_OK) {
      total_lines += fs.line_count;
      total_size  += fs.file_size;
    }
  }

  free(manifest);

  /* determine next checkpoint id */
  uint32_t next_id = lin_snap_latest_id(ctx, group) + 1;

  /* build snapshot */
  LinSnapshot snap;
  memset(&snap, 0, sizeof(snap));
  snap.id           = next_id;
  snap.timestamp_ms = lin_env_get_time();
  snap.total_lines  = total_lines;
  snap.total_size   = total_size;
  snap.total_files  = (uint32_t)file_count;
  snap.entry_count  = (uint32_t)file_count;
  snap.entries      = entries;

  if (message != NULL && message[0] != '\0') {
    snap.message_len = (uint16_t)strlen(message);
    snprintf(snap.message, sizeof(snap.message), "%s", message);
  }

  /* write binary file */
  char path[PATH_MAX];
  rc = snap_file_path(ctx, group, next_id, path, sizeof(path));
  if (rc != LIN_OK) {
    free(entries);
    return rc;
  }

  SnapWriteCtx wctx = { .snap = &snap };
  rc = lin_storage_write_atomic_fn(path, write_snap_cb, &wctx);

  free(entries);

  if (rc != LIN_OK)
    return rc;

  return (int)next_id;
}

int lin_snap_load(const LinContext *ctx, const char *group,
                  uint32_t id, LinSnapshot *snap)
{
  char path[PATH_MAX];
  int rc = snap_file_path(ctx, group, id, path, sizeof(path));
  if (rc != LIN_OK)
    return rc;

  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    lin_error_set(LIN_ERR_CHECKPOINT_NOT_FOUND,
                  "checkpoint %u: %s", id, strerror(errno));
    return LIN_ERR_CHECKPOINT_NOT_FOUND;
  }

  /* validate header */
  LinFileHeader hdr;
  rc = lin_format_read_header(fp, &hdr);
  if (rc != LIN_OK) {
    fclose(fp);
    return rc;
  }

  rc = lin_format_validate(&hdr, LIN_MAGIC_CKPT);
  if (rc != LIN_OK) {
    fclose(fp);
    return rc;
  }

  memset(snap, 0, sizeof(*snap));

  /* read checkpoint header fields */
  if (fread(&snap->id,           sizeof(snap->id),           1, fp) != 1 ||
      fread(&snap->timestamp_ms, sizeof(snap->timestamp_ms), 1, fp) != 1 ||
      fread(&snap->total_lines,  sizeof(snap->total_lines),  1, fp) != 1 ||
      fread(&snap->total_size,   sizeof(snap->total_size),   1, fp) != 1 ||
      fread(&snap->total_files,  sizeof(snap->total_files),  1, fp) != 1 ||
      fread(&snap->message_len,  sizeof(snap->message_len),  1, fp) != 1) {
    fclose(fp);
    lin_error_set(LIN_ERR_FORMAT_CORRUPT, "truncated checkpoint header");
    return LIN_ERR_FORMAT_CORRUPT;
  }

  /* read message */
  if (snap->message_len > 0) {
    if (snap->message_len > LIN_MESSAGE_MAX) {
      fclose(fp);
      lin_error_set(LIN_ERR_FORMAT_CORRUPT, "message length overflow");
      return LIN_ERR_FORMAT_CORRUPT;
    }
    if (fread(snap->message, 1, snap->message_len, fp) != snap->message_len) {
      fclose(fp);
      lin_error_set(LIN_ERR_FORMAT_CORRUPT, "truncated message");
      return LIN_ERR_FORMAT_CORRUPT;
    }
    snap->message[snap->message_len] = '\0';
  }

  /* read entry count */
  if (fread(&snap->entry_count, sizeof(snap->entry_count), 1, fp) != 1) {
    fclose(fp);
    lin_error_set(LIN_ERR_FORMAT_CORRUPT, "missing entry count");
    return LIN_ERR_FORMAT_CORRUPT;
  }

  /* allocate and read entries */
  snap->entries = NULL;
  if (snap->entry_count > 0) {
    snap->entries = calloc(snap->entry_count, sizeof(LinSnapEntry));
    if (snap->entries == NULL) {
      fclose(fp);
      lin_error_set(LIN_ERR_NOMEM, "snap entry allocation");
      return LIN_ERR_NOMEM;
    }

    for (uint32_t i = 0; i < snap->entry_count; i++) {
      LinSnapEntry *e = &snap->entries[i];

      if (fread(e->sha1,        LIN_SHA1_HEX_LEN,        1, fp) != 1 ||
          fread(&e->line_count, sizeof(e->line_count),    1, fp) != 1 ||
          fread(&e->file_size,  sizeof(e->file_size),     1, fp) != 1 ||
          fread(&e->path_len,   sizeof(e->path_len),      1, fp) != 1) {
        fclose(fp);
        lin_snap_free(snap);
        lin_error_set(LIN_ERR_FORMAT_CORRUPT, "truncated entry %u", i);
        return LIN_ERR_FORMAT_CORRUPT;
      }
      e->sha1[LIN_SHA1_HEX_LEN] = '\0';

      if (e->path_len > 0 && e->path_len < PATH_MAX) {
        if (fread(e->path, 1, e->path_len, fp) != e->path_len) {
          fclose(fp);
          lin_snap_free(snap);
          lin_error_set(LIN_ERR_FORMAT_CORRUPT,
                        "truncated path in entry %u", i);
          return LIN_ERR_FORMAT_CORRUPT;
        }
        e->path[e->path_len] = '\0';
      }
    }
  }

  fclose(fp);
  return LIN_OK;
}

uint32_t lin_snap_latest_id(const LinContext *ctx, const char *group)
{
  char dir[PATH_MAX];
  if (snap_dir_path(ctx, group, dir, sizeof(dir)) != LIN_OK)
    return 0;

  DIR *dp = opendir(dir);
  if (dp == NULL)
    return 0;

  uint32_t max_id = 0;
  struct dirent *de;

  while ((de = readdir(dp)) != NULL) {
    /* match pattern: NNNNNN.cp */
    size_t len = strlen(de->d_name);
    size_t ext_len = strlen(LIN_SNAP_EXT);

    if (len <= ext_len)
      continue;

    if (strcmp(de->d_name + len - ext_len, LIN_SNAP_EXT) != 0)
      continue;

    unsigned int id = 0;
    if (sscanf(de->d_name, "%u", &id) == 1 && id > max_id)
      max_id = id;
  }

  closedir(dp);
  return max_id;
}

int lin_snap_list(const LinContext *ctx, const char *group,
                  uint32_t *ids, int max_ids)
{
  char dir[PATH_MAX];
  int rc = snap_dir_path(ctx, group, dir, sizeof(dir));
  if (rc != LIN_OK)
    return rc;

  DIR *dp = opendir(dir);
  if (dp == NULL) {
    if (errno == ENOENT)
      return 0;
    lin_error_set(LIN_ERR_IO_OPEN, "opendir %s: %s", dir, strerror(errno));
    return LIN_ERR_IO_OPEN;
  }

  int count = 0;
  struct dirent *de;
  size_t ext_len = strlen(LIN_SNAP_EXT);

  while ((de = readdir(dp)) != NULL) {
    size_t len = strlen(de->d_name);
    if (len <= ext_len)
      continue;

    if (strcmp(de->d_name + len - ext_len, LIN_SNAP_EXT) != 0)
      continue;

    unsigned int id = 0;
    if (sscanf(de->d_name, "%u", &id) != 1)
      continue;

    if (count >= max_ids) {
      closedir(dp);
      lin_error_set(LIN_ERR_MANIFEST_FULL, "too many checkpoints");
      return LIN_ERR_MANIFEST_FULL;
    }

    ids[count++] = id;
  }

  closedir(dp);

  /* sort ascending */
  for (int i = 0; i < count - 1; i++) {
    for (int j = i + 1; j < count; j++) {
      if (ids[j] < ids[i]) {
        uint32_t tmp = ids[i];
        ids[i] = ids[j];
        ids[j] = tmp;
      }
    }
  }

  return count;
}

void lin_snap_free(LinSnapshot *snap)
{
  if (snap == NULL)
    return;

  free(snap->entries);
  snap->entries     = NULL;
  snap->entry_count = 0;
}
