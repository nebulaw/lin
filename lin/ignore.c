#include "ignore.h"
#include "error.h"

#include <fnmatch.h>
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  built-in default patterns                                          */
/* ------------------------------------------------------------------ */

static const char *default_patterns[] = {
  /* version control */
  ".git",
  ".lin",
  ".svn",
  ".hg",

  /* build artifacts */
  "build",
  "dist",
  "target",
  "bin",
  "obj",

  /* dependencies */
  "node_modules",
  "vendor",
  "__pycache__",
  ".venv",
  "venv",

  /* compiled objects */
  "*.o",
  "*.so",
  "*.dylib",
  "*.a",
  "*.dll",
  "*.exe",
  "*.class",
  "*.pyc",

  /* binary media */
  "*.jpg",
  "*.jpeg",
  "*.png",
  "*.gif",
  "*.ico",
  "*.pdf",
  "*.zip",
  "*.tar",
  "*.gz",
  "*.bz2",

  /* IDE/editor */
  ".idea",
  ".vscode",
  "*.swp",
  "*.swo",
  ".DS_Store",

  NULL,
};

/* ------------------------------------------------------------------ */
/*  helpers                                                            */
/* ------------------------------------------------------------------ */

static void add_pattern(LinIgnore *ign, const char *pat, bool negated)
{
  if (ign->count >= LIN_IGNORE_MAX)
    return;

  snprintf(ign->patterns[ign->count], LIN_IGNORE_PAT_MAX, "%s", pat);
  ign->negated[ign->count] = negated;
  ign->count++;
}

static void load_defaults(LinIgnore *ign)
{
  for (int i = 0; default_patterns[i] != NULL; i++)
    add_pattern(ign, default_patterns[i], false);
}

static int load_file(LinIgnore *ign, const char *path)
{
  FILE *fp = fopen(path, "r");
  if (fp == NULL)
    return LIN_OK; /* file not found is fine */

  char line[LIN_IGNORE_PAT_MAX];

  while (fgets(line, sizeof(line), fp) != NULL) {
    /* strip newline */
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
      line[--len] = '\0';

    /* skip empty lines and comments */
    if (len == 0 || line[0] == '#')
      continue;

    /* strip trailing whitespace */
    while (len > 0 && line[len - 1] == ' ')
      line[--len] = '\0';

    if (len == 0)
      continue;

    /* negation: leading '!' means unignore */
    bool neg = false;
    const char *pat = line;
    if (pat[0] == '!') {
      neg = true;
      pat++;
    }

    add_pattern(ign, pat, neg);
  }

  fclose(fp);
  return LIN_OK;
}

/* ------------------------------------------------------------------ */
/*  public API                                                         */
/* ------------------------------------------------------------------ */

int lin_ignore_load(const LinContext *ctx, LinIgnore *ign)
{
  memset(ign, 0, sizeof(*ign));

  load_defaults(ign);

  /* load project-root .linignore */
  if (ctx->root_dir[0] != '\0') {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/" LIN_IGNORE_FILE, ctx->root_dir);
    load_file(ign, path);
  }

  return LIN_OK;
}

/*
 * Check if a pattern matches any component of the path.
 * e.g. pattern "node_modules" matches "node_modules/dep.js"
 * because "node_modules" is a path component.
 */
static bool match_any_component(const char *pat, const char *path)
{
  char buf[PATH_MAX];
  snprintf(buf, sizeof(buf), "%s", path);

  char *tok = buf;
  char *sep;

  while ((sep = strchr(tok, '/')) != NULL) {
    *sep = '\0';
    if (fnmatch(pat, tok, 0) == 0)
      return true;
    tok = sep + 1;
  }

  /* check the last component (filename) */
  if (fnmatch(pat, tok, 0) == 0)
    return true;

  return false;
}

bool lin_ignore_match(const LinIgnore *ign, const char *path)
{
  bool ignored = false;

  for (int i = 0; i < ign->count; i++) {
    const char *pat = ign->patterns[i];

    /* match against full path and each path component */
    int match = (fnmatch(pat, path, 0) == 0 ||
                 match_any_component(pat, path));

    if (match) {
      if (ign->negated[i])
        ignored = false;
      else
        ignored = true;
    }
  }

  return ignored;
}
