#ifndef LIN_FORMAT_H
#define LIN_FORMAT_H

#include <stdint.h>
#include <stdio.h>

/*
 * Every binary file under .lin/ starts with an 8-byte header
 * that identifies the file type and the format version.
 *
 * Binary data uses the host's native byte order.  Files are
 * not intended to be transferred between architectures.
 */

#define LIN_MAGIC_INFO 0x4C494E46 /* "LINF" - group info   */
#define LIN_MAGIC_CKPT 0x4C494E43 /* "LINC" - checkpoint   */
#define LIN_MAGIC_CONF 0x4C494E53 /* "LINS" - config       */

#define LIN_FORMAT_V1  1

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint16_t flags;     /* reserved, must be 0 */
} LinFileHeader;

int lin_format_write_header(FILE *fp, uint32_t magic, uint16_t version);
int lin_format_read_header(FILE *fp, LinFileHeader *out);
int lin_format_validate(const LinFileHeader *hdr, uint32_t expected_magic);

#endif /* LIN_FORMAT_H */
