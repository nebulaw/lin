#include "env.h"
#include "add.h"
#include "objects.h"
#include "enc.h"
#include "linio.h"

#include <sys/stat.h>
#include <unistd.h>


static struct stat temp_st = {0};
static char hashbuf[64];


void lin_cmd_execute_add(int argc, int argvi, char **argv)
{
  if (argc <= argvi) {
    fprintf(stderr, "lin: please specify a file.\n");
    exit(L_FAILURE);
  }

  // check if files exist
  for (int i = argvi; i < argc; i++) {
    if (!lin_io_path_exists(argv[i])) {
      fprintf(stderr, "lin: file %s not found.\n", argv[i]);
      exit(L_FAILURE);
    }
  }

  while (argvi < argc) {
    lin_add_file(argv[argvi]);
    argvi++;
  }

  exit(L_SUCCESS);
}

int lin_add_file(const char *file_name)
{
  int ret_st = L_ADD_FAILURE;
  FILE *file;
  FILE *object_file = NULL;

  file = fopen(file_name, "r");
  if (file == NULL) {
    return L_FILE_NOT_FOUND;
  }
  hashbuf[0] = '\0';
  lin_hash_sha1sum_from_file(hashbuf, file);
  fprintf(stdout, "%s\t%s\n", hashbuf, file_name);
  int rst = lin_object_create(lin_env_group, hashbuf, object_file, 1);
  if (rst == L_OBJECT_CREATED) {
    fprintf(stdout, "lin: object created.\n");
    ret_st = L_ADD_SUCCESS;
  } else {
    fprintf(stderr, "lin: object not created.\n");
  }
  hashbuf[0] = '\0';
  return ret_st;
}
