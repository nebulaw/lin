#include "env.h"
#include "objects.h"

#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>


static struct stat temp_st = {0};
static char pathbuf[PATH_MAX];

void lin_object_inner_group_from_str(char dest[INNER_GROUP_LEN], const char *src)
{
  strncpy(dest, src, 6);
  dest[INNER_GROUP_LEN - 1] = '\0';
}

int lin_object_inner_group_merge(const char group_name[GROUP_MAX_LEN], const char inner_group[INNER_GROUP_LEN])
{
  int ret_st = 0;
  // reset pathbuf
  pathbuf[0] = '\0';
  snprintf(pathbuf, sizeof(pathbuf), ".lin/%s/objects/%s", group_name, inner_group);
  // if inner group does not exist, create it
  if (stat(pathbuf, &temp_st) == -1) {
    if (mkdir(pathbuf, 0755) != 0) {
      ret_st = 1;
    }
  }
  // reset pathbuf
  pathbuf[0] = '\0';
  return ret_st;
}

int lin_object_create(const char *group_name, const char *object_name, FILE *fptr, int append)
{
  if (strlen(object_name) < 7) {
    return L_OBJECT_NAME_TOO_SHORT;
  }
  // assume group already exists
  char inner_group[7];
  // create inner group
  lin_object_inner_group_from_str(inner_group, object_name);
  // merge resets pathbuf
  if (lin_object_inner_group_merge(lin_env_group, inner_group) != 0) {
    fprintf(stderr, "lin: inner group %s not created\n", inner_group);
    return L_OBJECT_NOT_CREATED;
  }
  // create object
  pathbuf[0] = '\0';
  snprintf(pathbuf, sizeof(pathbuf), ".lin/%s/objects/%s/%s", lin_env_group, inner_group, object_name + (INNER_GROUP_LEN - 1));
  fprintf(stdout, "lin: %s\n", pathbuf);
  if (stat(pathbuf, &temp_st) == 0) {
    // create binary file for writing
    fptr = fopen(pathbuf, append ? "rb+" : "wb+");
  } else {
    fptr = fopen(pathbuf, append ? "ab+" : "wb+");
    fprintf(stdout, "lin: file opened\n");
    if (fptr == NULL) {
      fprintf(stderr, "lin: fptr is null\n");
    }
  }
  pathbuf[0] = '\0';
  return fptr == NULL ? L_OBJECT_NOT_CREATED : L_OBJECT_CREATED;
}

int lin_object_exists(const char *group_name, const char *object_name)
{
  int ret_st;
  char inner_group[7];
  lin_object_inner_group_from_str(inner_group, object_name);
  pathbuf[0] = '\0';
  snprintf(pathbuf, sizeof(pathbuf), ".lin/%s/objects/%s/%s", group_name, inner_group, object_name + (INNER_GROUP_LEN - 1));
  ret_st = stat(pathbuf, &temp_st);
  pathbuf[0] = '\0';
  return ret_st == 0 ? 1 : 0;
}

int lin_object_remove(const char *group_name, const char *object_name)
{
  int ret_st = L_OBJECT_NOT_REMOVED;
  char inner_group[7];
  lin_object_inner_group_from_str(inner_group, object_name);
  pathbuf[0] = '\0';
  snprintf(pathbuf, sizeof(pathbuf), ".lin/%s/objects/%s/%s", group_name, inner_group, object_name + (INNER_GROUP_LEN - 1));
  if (stat(pathbuf, &temp_st) == 0) {
    if (remove(pathbuf) == 0) {
      ret_st = L_OBJECT_REMOVED;
    }
  } else {
    ret_st = L_OBJECT_NOT_FOUND;
  }
  pathbuf[0] = '\0';
  return ret_st;
}
