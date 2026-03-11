#ifndef LIN_IGNORE_H
#define LIN_IGNORE_H

#include "context.h"

#include <stdbool.h>

#define LIN_IGNORE_FILE     ".linignore"
#define LIN_IGNORE_MAX      512
#define LIN_IGNORE_PAT_MAX  256

typedef struct {
  char patterns[LIN_IGNORE_MAX][LIN_IGNORE_PAT_MAX];
  bool negated[LIN_IGNORE_MAX];
  int  count;
} LinIgnore;

/*
 * Load ignore patterns from .linignore (project root)
 * and built-in defaults.
 */
int lin_ignore_load(const LinContext *ctx, LinIgnore *ign);

/*
 * Test whether a path should be ignored.
 * Returns true if the path matches an ignore pattern.
 */
bool lin_ignore_match(const LinIgnore *ign, const char *path);

#endif /* LIN_IGNORE_H */
