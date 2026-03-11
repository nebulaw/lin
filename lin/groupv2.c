#include "groupv2.h"
#include "error.h"
#include "storage.h"
#include "linio.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* ------------------------------------------------------------------ */
/*  path helpers                                                       */
/* ------------------------------------------------------------------ */

static int group_dir_path(const LinContext *ctx, const char *name,
                          char *out, size_t size)
{
  return lin_context_path(ctx, out, size, "/%s", name);
}

static int group_info_path(const LinContext *ctx, const char *name,
                           char *out, size_t size)
{
  return lin_context_path(ctx, out, size,
                          "/%s/" LIN_GROUP_INFO_FILE, name);
}

/* ------------------------------------------------------------------ */
/*  info binary serialization                                          */
/* ------------------------------------------------------------------ */

typedef struct {
  const LinGroupInfo *info;
} InfoWriteCtx;

static int write_info_cb(FILE *fp, void *udata)
{
  InfoWriteCtx *wctx = udata;
  const LinGroupInfo *gi = wctx->info;

  int rc = lin_format_write_header(fp, LIN_MAGIC_INFO, LIN_FORMAT_V1);
  if (rc != LIN_OK)
    return rc;

  if (fwrite(&gi->created_ms,       sizeof(gi->created_ms),       1, fp) != 1 ||
      fwrite(&gi->updated_ms,       sizeof(gi->updated_ms),       1, fp) != 1 ||
      fwrite(&gi->total_lines,      sizeof(gi->total_lines),      1, fp) != 1 ||
      fwrite(&gi->total_files,      sizeof(gi->total_files),      1, fp) != 1 ||
      fwrite(&gi->total_checkpoints,sizeof(gi->total_checkpoints),1, fp) != 1) {
    lin_error_set(LIN_ERR_IO_WRITE, "failed to write group info");
    return LIN_ERR_IO_WRITE;
  }

  /* write group name length + name */
  uint32_t name_len = (uint32_t)strlen(gi->name);
  if (fwrite(&name_len, sizeof(name_len), 1, fp) != 1 ||
      fwrite(gi->name, 1, name_len, fp) != name_len) {
    lin_error_set(LIN_ERR_IO_WRITE, "failed to write group name");
    return LIN_ERR_IO_WRITE;
  }

  return LIN_OK;
}

/* ------------------------------------------------------------------ */
/*  public API                                                         */
/* ------------------------------------------------------------------ */

int lin_groupv2_create(const LinContext *ctx, const char *name)
{
  if (lin_groupv2_exists(ctx, name))
    return LIN_ERR_GROUP_EXISTS;

  char dir[PATH_MAX];
  int rc = group_dir_path(ctx, name, dir, sizeof(dir));
  if (rc != LIN_OK)
    return rc;

  /* create group directory */
  rc = lin_storage_ensure_dir(dir, 0755);
  if (rc != LIN_OK)
    return rc;

  /* create checkpoints subdirectory */
  char cp_dir[PATH_MAX];
  snprintf(cp_dir, sizeof(cp_dir), "%s/checkpoints", dir);
  rc = lin_storage_ensure_dir(cp_dir, 0755);
  if (rc != LIN_OK)
    return rc;

  /* write initial info file */
  LinGroupInfo info;
  memset(&info, 0, sizeof(info));
  info.created_ms = lin_env_get_time();
  info.updated_ms = info.created_ms;
  snprintf(info.name, sizeof(info.name), "%s", name);

  rc = lin_groupv2_info_write(ctx, name, &info);
  if (rc != LIN_OK)
    return rc;

  return LIN_OK;
}

int lin_groupv2_remove(const LinContext *ctx, const char *name)
{
  char dir[PATH_MAX];
  int rc = group_dir_path(ctx, name, dir, sizeof(dir));
  if (rc != LIN_OK)
    return rc;

  if (!lin_groupv2_exists(ctx, name)) {
    lin_error_set(LIN_ERR_GROUP_NOT_FOUND, "group '%s' not found", name);
    return LIN_ERR_GROUP_NOT_FOUND;
  }

  if (lin_io_path_remove(dir) != 0) {
    lin_error_set(LIN_ERR_IO_REMOVE, "failed to remove group '%s'", name);
    return LIN_ERR_IO_REMOVE;
  }

  return LIN_OK;
}

int lin_groupv2_exists(const LinContext *ctx, const char *name)
{
  char path[PATH_MAX];
  if (group_dir_path(ctx, name, path, sizeof(path)) != LIN_OK)
    return 0;
  return lin_io_path_exists(path);
}

int lin_groupv2_info_read(const LinContext *ctx, const char *name,
                          LinGroupInfo *info)
{
  char path[PATH_MAX];
  int rc = group_info_path(ctx, name, path, sizeof(path));
  if (rc != LIN_OK)
    return rc;

  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    lin_error_set(LIN_ERR_IO_OPEN, "open %s: %s", path, strerror(errno));
    return LIN_ERR_IO_OPEN;
  }

  /* validate header */
  LinFileHeader hdr;
  rc = lin_format_read_header(fp, &hdr);
  if (rc != LIN_OK) {
    fclose(fp);
    return rc;
  }

  rc = lin_format_validate(&hdr, LIN_MAGIC_INFO);
  if (rc != LIN_OK) {
    fclose(fp);
    return rc;
  }

  memset(info, 0, sizeof(*info));

  if (fread(&info->created_ms,        sizeof(info->created_ms),        1, fp) != 1 ||
      fread(&info->updated_ms,        sizeof(info->updated_ms),        1, fp) != 1 ||
      fread(&info->total_lines,       sizeof(info->total_lines),       1, fp) != 1 ||
      fread(&info->total_files,       sizeof(info->total_files),       1, fp) != 1 ||
      fread(&info->total_checkpoints, sizeof(info->total_checkpoints), 1, fp) != 1) {
    fclose(fp);
    lin_error_set(LIN_ERR_FORMAT_CORRUPT, "truncated group info");
    return LIN_ERR_FORMAT_CORRUPT;
  }

  /* read group name */
  uint32_t name_len = 0;
  if (fread(&name_len, sizeof(name_len), 1, fp) == 1 && name_len <= LIN_GROUP_MAX) {
    if (fread(info->name, 1, name_len, fp) == name_len)
      info->name[name_len] = '\0';
  }

  fclose(fp);
  return LIN_OK;
}

int lin_groupv2_info_write(const LinContext *ctx, const char *name,
                           const LinGroupInfo *info)
{
  char path[PATH_MAX];
  int rc = group_info_path(ctx, name, path, sizeof(path));
  if (rc != LIN_OK)
    return rc;

  InfoWriteCtx wctx = { .info = info };
  return lin_storage_write_atomic_fn(path, write_info_cb, &wctx);
}

int lin_groupv2_list(const LinContext *ctx,
                     char names[][LIN_GROUP_MAX + 1], int max)
{
  DIR *dp = opendir(ctx->lin_dir);
  if (dp == NULL) {
    lin_error_set(LIN_ERR_IO_OPEN, "opendir %s: %s",
                  ctx->lin_dir, strerror(errno));
    return LIN_ERR_IO_OPEN;
  }

  int count = 0;
  struct dirent *de;

  while ((de = readdir(dp)) != NULL) {
    if (de->d_name[0] == '.')
      continue;

    /* only include directories that contain an info file */
    char info_path[PATH_MAX];
    int rc = lin_context_path(ctx, info_path, sizeof(info_path),
                              "/%s/" LIN_GROUP_INFO_FILE, de->d_name);
    if (rc != LIN_OK)
      continue;

    if (!lin_io_path_exists(info_path))
      continue;

    if (count >= max)
      break;

    snprintf(names[count], LIN_GROUP_MAX + 1, "%s", de->d_name);
    count++;
  }

  closedir(dp);
  return count;
}
