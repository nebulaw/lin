#ifndef LIN_HELP_H
#define LIN_HELP_H

#include <stdio.h>
#include <string.h>

// [program_name]_[module]_[action]_[object]

void lin_cmd_execute_help(int argc, int argvi, char **argv);
void lin_help();
void lin_help_group();
void lin_help_add();

#endif // LIN_HELP_H
