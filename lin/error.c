#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static LinError current_error = {.code = LIN_OK, .message = {0}};

void lin_error_set(int code, const char *fmt, ...)
{
  current_error.code = code;

  if (fmt != NULL) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(current_error.message, sizeof(current_error.message), fmt, ap);
    va_end(ap);
  } else {
    current_error.message[0] = '\0';
  }
}

const LinError *lin_error_get(void)
{
  return &current_error;
}

void lin_error_clear(void)
{
  current_error.code = LIN_OK;
  current_error.message[0] = '\0';
}

const char *lin_error_code_str(int code)
{
  switch (code) {
  case LIN_OK:                          return "success";
  case LIN_ERR:                         return "general error";
  case LIN_ERR_NOMEM:                   return "out of memory";
  case LIN_ERR_INVAL:                   return "invalid argument";
  case LIN_ERR_IO:                      return "i/o error";
  case LIN_ERR_IO_OPEN:                 return "failed to open file";
  case LIN_ERR_IO_READ:                 return "failed to read file";
  case LIN_ERR_IO_WRITE:                return "failed to write file";
  case LIN_ERR_IO_MKDIR:                return "failed to create directory";
  case LIN_ERR_IO_REMOVE:               return "failed to remove path";
  case LIN_ERR_IO_RENAME:               return "failed to rename file";
  case LIN_ERR_IO_FSYNC:                return "failed to sync file";
  case LIN_ERR_IO_LOCK:                 return "failed to acquire lock";
  case LIN_ERR_PATH:                    return "path error";
  case LIN_ERR_PATH_NOT_FOUND:          return "path not found";
  case LIN_ERR_PATH_EXISTS:             return "path already exists";
  case LIN_ERR_PATH_TOO_LONG:           return "path exceeds maximum length";
  case LIN_ERR_PATH_INVALID:            return "invalid path";
  case LIN_ERR_FORMAT:                  return "format error";
  case LIN_ERR_FORMAT_MAGIC:            return "invalid file magic bytes";
  case LIN_ERR_FORMAT_VERSION:          return "unsupported format version";
  case LIN_ERR_FORMAT_CORRUPT:          return "corrupt file data";
  case LIN_ERR_GROUP:                   return "group error";
  case LIN_ERR_GROUP_NOT_FOUND:         return "group not found";
  case LIN_ERR_GROUP_EXISTS:            return "group already exists";
  case LIN_ERR_GROUP_NAME:              return "invalid group name";
  case LIN_ERR_MANIFEST:                return "manifest error";
  case LIN_ERR_MANIFEST_PARSE:          return "failed to parse manifest";
  case LIN_ERR_MANIFEST_FULL:           return "manifest capacity exceeded";
  case LIN_ERR_MANIFEST_ENTRY_EXISTS:   return "manifest entry already exists";
  case LIN_ERR_MANIFEST_ENTRY_NOT_FOUND:return "manifest entry not found";
  case LIN_ERR_CHECKPOINT:              return "checkpoint error";
  case LIN_ERR_CHECKPOINT_NOT_FOUND:    return "checkpoint not found";
  case LIN_ERR_INIT:                    return "initialization error";
  case LIN_ERR_INIT_EXISTS:             return ".lin directory already exists";
  case LIN_ERR_CTX:                     return "context error";
  case LIN_ERR_CTX_NO_ROOT:             return ".lin directory not found";
  case LIN_ERR_CTX_GROUP:               return "invalid group specification";
  case LIN_ERR_CTX_MESSAGE:             return "invalid message specification";
  default:                              return "unknown error";
  }
}
