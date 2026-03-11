#ifndef LIN_HELP_H
#define LIN_HELP_H

#include "context.h"

void lin_cmd_execute_help(LinContext *ctx, int argc, int argvi, char **argv);
void lin_help_print(void);

#endif /* LIN_HELP_H */
