#ifndef LIN_INIT_H_
#define LIN_INIT_H_

void lin_cmd_execute_init(int argc, int argvi, char **argv);
int lin_dot_dir_initialize(void);
int lin_dot_dir_reinitialize(void);
int lin_dot_dir_exists(void);

#endif // LIN_INIT_H_