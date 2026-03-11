#include "format.h"
#include "error.h"

#include <string.h>

int lin_format_write_header(FILE *fp, uint32_t magic, uint16_t version)
{
  LinFileHeader hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.magic   = magic;
  hdr.version = version;
  hdr.flags   = 0;

  if (fwrite(&hdr, sizeof(hdr), 1, fp) != 1) {
    lin_error_set(LIN_ERR_IO_WRITE, "failed to write file header");
    return LIN_ERR_IO_WRITE;
  }

  return LIN_OK;
}

int lin_format_read_header(FILE *fp, LinFileHeader *out)
{
  if (fread(out, sizeof(*out), 1, fp) != 1) {
    lin_error_set(LIN_ERR_IO_READ, "failed to read file header");
    return LIN_ERR_IO_READ;
  }

  return LIN_OK;
}

int lin_format_validate(const LinFileHeader *hdr, uint32_t expected_magic)
{
  if (hdr->magic != expected_magic) {
    lin_error_set(LIN_ERR_FORMAT_MAGIC,
                  "expected magic 0x%08X, got 0x%08X",
                  expected_magic, hdr->magic);
    return LIN_ERR_FORMAT_MAGIC;
  }

  if (hdr->version < 1 || hdr->version > LIN_FORMAT_V1) {
    lin_error_set(LIN_ERR_FORMAT_VERSION,
                  "unsupported format version %u (max %u)",
                  hdr->version, LIN_FORMAT_V1);
    return LIN_ERR_FORMAT_VERSION;
  }

  return LIN_OK;
}
