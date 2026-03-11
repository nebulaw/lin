#ifndef LIN_CONTEXT_H
#define LIN_CONTEXT_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#define LIN_DIR_NAME    ".lin"
#define LIN_GROUP_MAX   24
#define LIN_MESSAGE_MAX 200

typedef struct {
  char group[LIN_GROUP_MAX + 1];
  char message[LIN_MESSAGE_MAX + 1];
  char root_dir[PATH_MAX];   /* project root (parent of .lin/) */
  char lin_dir[PATH_MAX];    /* absolute path to .lin/         */
  bool verbose;
  bool dry_run;
  bool group_set;            /* true if -g was provided        */
  bool message_set;          /* true if -m was provided        */
} LinContext;

/*
 * Zero the context and set safe defaults.
 * Does NOT resolve the .lin directory.
 */
int  lin_context_init(LinContext *ctx);
void lin_context_destroy(LinContext *ctx);

/*
 * Walk from cwd upward to locate the nearest .lin/ directory.
 * Fills ctx->root_dir and ctx->lin_dir on success.
 */
int lin_context_resolve_root(LinContext *ctx);

int lin_context_set_group(LinContext *ctx, const char *name);
int lin_context_set_message(LinContext *ctx, const char *msg);

/*
 * Build a path relative to ctx->lin_dir.
 *
 *   lin_context_path(ctx, buf, sizeof(buf), "/%s/info", grp);
 *   => "/absolute/path/to/.lin/mygroup/info"
 */
int lin_context_path(const LinContext *ctx, char *out, size_t size,
                     const char *fmt, ...);

#endif /* LIN_CONTEXT_H */
