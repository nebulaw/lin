#include "context.h"
#include "error.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/*  validation                                                         */
/* ------------------------------------------------------------------ */

/*
 * Allowed characters: a-z  A-Z  0-9  -  _
 * Rejects: empty, ".", "..", contains '/', length > LIN_GROUP_MAX
 */
static int is_valid_group_name(const char *name)
{
  if (name == NULL || name[0] == '\0')
    return 0;

  size_t len = strlen(name);
  if (len > LIN_GROUP_MAX)
    return 0;

  if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
    return 0;

  for (size_t i = 0; i < len; i++) {
    char c = name[i];
    if (!isalnum((unsigned char)c) && c != '-' && c != '_')
      return 0;
  }

  return 1;
}

/* ------------------------------------------------------------------ */
/*  public API                                                         */
/* ------------------------------------------------------------------ */

int lin_context_init(LinContext *ctx)
{
  if (ctx == NULL) {
    lin_error_set(LIN_ERR_INVAL, "null context pointer");
    return LIN_ERR_INVAL;
  }

  memset(ctx, 0, sizeof(*ctx));

  /* safe default: commands fall back to "default" group */
  memcpy(ctx->group, "default", 7);
  ctx->verbose     = false;
  ctx->dry_run     = false;
  ctx->group_set   = false;
  ctx->message_set = false;

  return LIN_OK;
}

void lin_context_destroy(LinContext *ctx)
{
  if (ctx != NULL)
    memset(ctx, 0, sizeof(*ctx));
}

int lin_context_resolve_root(LinContext *ctx)
{
  char path[PATH_MAX];
  char check[PATH_MAX];
  struct stat st;

  if (getcwd(path, sizeof(path)) == NULL) {
    lin_error_set(LIN_ERR_CTX_NO_ROOT, "getcwd failed");
    return LIN_ERR_CTX_NO_ROOT;
  }

  for (;;) {
    snprintf(check, sizeof(check), "%s/" LIN_DIR_NAME, path);

    if (stat(check, &st) == 0 && S_ISDIR(st.st_mode)) {
      memcpy(ctx->root_dir, path, PATH_MAX);
      memcpy(ctx->lin_dir, check, PATH_MAX);
      return LIN_OK;
    }

    /* move up one directory */
    char *sep = strrchr(path, '/');
    if (sep == NULL)
      break;

    if (sep == path) {
      /* check the root directory itself */
      snprintf(check, sizeof(check), "/" LIN_DIR_NAME);
      if (stat(check, &st) == 0 && S_ISDIR(st.st_mode)) {
        ctx->root_dir[0] = '/';
        ctx->root_dir[1] = '\0';
        memcpy(ctx->lin_dir, check, PATH_MAX);
        return LIN_OK;
      }
      break;
    }

    *sep = '\0';
  }

  lin_error_set(LIN_ERR_CTX_NO_ROOT,
                LIN_DIR_NAME " directory not found in any parent directory");
  return LIN_ERR_CTX_NO_ROOT;
}

int lin_context_set_group(LinContext *ctx, const char *name)
{
  if (!is_valid_group_name(name)) {
    lin_error_set(LIN_ERR_CTX_GROUP,
                  "invalid group name '%s' "
                  "(use a-z, A-Z, 0-9, '-', '_'; max %d chars)",
                  name ? name : "(null)", LIN_GROUP_MAX);
    return LIN_ERR_CTX_GROUP;
  }

  snprintf(ctx->group, sizeof(ctx->group), "%s", name);
  ctx->group_set = true;
  return LIN_OK;
}

int lin_context_set_message(LinContext *ctx, const char *msg)
{
  if (msg == NULL) {
    lin_error_set(LIN_ERR_CTX_MESSAGE, "null message");
    return LIN_ERR_CTX_MESSAGE;
  }

  size_t len = strlen(msg);
  if (len > LIN_MESSAGE_MAX) {
    lin_error_set(LIN_ERR_CTX_MESSAGE,
                  "message too long (%zu chars, max %d)", len, LIN_MESSAGE_MAX);
    return LIN_ERR_CTX_MESSAGE;
  }

  snprintf(ctx->message, sizeof(ctx->message), "%s", msg);
  ctx->message_set = true;
  return LIN_OK;
}

int lin_context_path(const LinContext *ctx, char *out, size_t size,
                     const char *fmt, ...)
{
  int base = snprintf(out, size, "%s", ctx->lin_dir);
  if (base < 0 || (size_t)base >= size) {
    lin_error_set(LIN_ERR_PATH_TOO_LONG, "base path exceeds buffer");
    return LIN_ERR_PATH_TOO_LONG;
  }

  va_list ap;
  va_start(ap, fmt);
  int rest = vsnprintf(out + base, size - (size_t)base, fmt, ap);
  va_end(ap);

  if (rest < 0 || (size_t)(base + rest) >= size) {
    lin_error_set(LIN_ERR_PATH_TOO_LONG, "full path exceeds buffer");
    return LIN_ERR_PATH_TOO_LONG;
  }

  return LIN_OK;
}
