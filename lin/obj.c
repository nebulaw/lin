#include "env.h"
#include "obj.h"

#include <stdio.h>


static char obj_file_path_buf[OBJ_FILE_MAX_LEN];

int lin_obj_file_create(const char *file_path) {
  struct stat obj_file_st = {0};
  FILE *obj_file;
  snprintf(obj_file_path_buf, sizeof(obj_file_path_buf), ".lin/%s", lin_env_group);


}

int lin_obj_write_object(const char *dst_path, void *object, size_t object_size) {
  FILE *dst_file = fopen(dst_path, "wb");

}
