#ifndef LIN_ERROR_H
#define LIN_ERROR_H

#define LIN_ERROR_MSG_MAX 512

/*
 * Return code convention:
 *   LIN_OK (0)   = success
 *   negative     = error
 *
 * Code ranges are grouped by subsystem so new codes
 * can be added without renumbering existing ones.
 */
#define LIN_OK 0

/* General (-1 .. -99) */
#define LIN_ERR                          -1
#define LIN_ERR_NOMEM                    -2
#define LIN_ERR_INVAL                    -3

/* I/O (-100 .. -199) */
#define LIN_ERR_IO                       -100
#define LIN_ERR_IO_OPEN                  -101
#define LIN_ERR_IO_READ                  -102
#define LIN_ERR_IO_WRITE                 -103
#define LIN_ERR_IO_MKDIR                 -104
#define LIN_ERR_IO_REMOVE                -105
#define LIN_ERR_IO_RENAME                -106
#define LIN_ERR_IO_FSYNC                 -107
#define LIN_ERR_IO_LOCK                  -108

/* Path (-200 .. -299) */
#define LIN_ERR_PATH                     -200
#define LIN_ERR_PATH_NOT_FOUND           -201
#define LIN_ERR_PATH_EXISTS              -202
#define LIN_ERR_PATH_TOO_LONG            -203
#define LIN_ERR_PATH_INVALID             -204

/* Binary format (-300 .. -399) */
#define LIN_ERR_FORMAT                   -300
#define LIN_ERR_FORMAT_MAGIC             -301
#define LIN_ERR_FORMAT_VERSION           -302
#define LIN_ERR_FORMAT_CORRUPT           -303

/* Group (-400 .. -499) */
#define LIN_ERR_GROUP                    -400
#define LIN_ERR_GROUP_NOT_FOUND          -401
#define LIN_ERR_GROUP_EXISTS             -402
#define LIN_ERR_GROUP_NAME               -403

/* Manifest (-500 .. -599) */
#define LIN_ERR_MANIFEST                 -500
#define LIN_ERR_MANIFEST_PARSE           -501
#define LIN_ERR_MANIFEST_FULL            -502
#define LIN_ERR_MANIFEST_ENTRY_EXISTS    -503
#define LIN_ERR_MANIFEST_ENTRY_NOT_FOUND -504

/* Checkpoint (-600 .. -699) */
#define LIN_ERR_CHECKPOINT               -600
#define LIN_ERR_CHECKPOINT_NOT_FOUND     -601

/* Init (-700 .. -799) */
#define LIN_ERR_INIT                     -700
#define LIN_ERR_INIT_EXISTS              -701

/* Context (-800 .. -899) */
#define LIN_ERR_CTX                      -800
#define LIN_ERR_CTX_NO_ROOT              -801
#define LIN_ERR_CTX_GROUP                -802
#define LIN_ERR_CTX_MESSAGE              -803

typedef struct {
  int  code;
  char message[LIN_ERROR_MSG_MAX];
} LinError;

void            lin_error_set(int code, const char *fmt, ...);
const LinError *lin_error_get(void);
void            lin_error_clear(void);
const char     *lin_error_code_str(int code);

#endif /* LIN_ERROR_H */
