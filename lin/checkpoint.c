#include "env.h"
#include "checkpoint.h"
#include "linio.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

void lin_cmd_execute_checkpoint(int argc, int argvi, char **argv) {
  struct stat path_st = {0};
  unsigned long total_lines = 0;

  int st;
  while (argvi < argc) {
    if (stat(argv[argvi], &path_st) == 0) {
      if (S_ISREG(path_st.st_mode)) {
        unsigned long lines = lin_io_count_lines(argv[argvi]);
        fprintf(stdout, "%5lu: %s.\n", lines, argv[argvi]);
        total_lines += lines;
      }
    } else {
      fprintf(stderr, "%s: not found.\n", argv[argvi]);
    }
    argvi++;
  }
  fprintf(stdout, "-------------------------------\n");
  fprintf(stdout, "%5lu: total lines.\n", total_lines);
}

int lin_checkpoint_create(const char *path) {

}
