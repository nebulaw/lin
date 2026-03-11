#include "analysis.h"
#include "error.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/evp.h>

#define SCAN_BUF_SIZE 8192

int lin_analysis_scan_file(const char *path, LinFileStats *stats)
{
  struct stat st;
  if (stat(path, &st) != 0) {
    stats->exists = false;
    lin_error_set(LIN_ERR_PATH_NOT_FOUND, "%s: %s", path, strerror(errno));
    return LIN_ERR_PATH_NOT_FOUND;
  }

  if (!S_ISREG(st.st_mode)) {
    lin_error_set(LIN_ERR_PATH_INVALID, "%s: not a regular file", path);
    return LIN_ERR_PATH_INVALID;
  }

  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    stats->exists = true;
    lin_error_set(LIN_ERR_IO_OPEN, "%s: %s", path, strerror(errno));
    return LIN_ERR_IO_OPEN;
  }

  EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
  if (md_ctx == NULL) {
    fclose(fp);
    lin_error_set(LIN_ERR, "failed to create hash context");
    return LIN_ERR;
  }

  const EVP_MD *md = EVP_sha1();
  EVP_DigestInit_ex(md_ctx, md, NULL);

  unsigned char buf[SCAN_BUF_SIZE];
  uint64_t lines = 0;
  bool binary = false;
  size_t nread;

  /* single pass: hash, count lines, detect binary */
  while ((nread = fread(buf, 1, sizeof(buf), fp)) > 0) {
    EVP_DigestUpdate(md_ctx, buf, nread);

    for (size_t i = 0; i < nread; i++) {
      if (buf[i] == '\n')
        lines++;
      else if (buf[i] == '\0')
        binary = true;
    }
  }

  if (ferror(fp)) {
    EVP_MD_CTX_free(md_ctx);
    fclose(fp);
    lin_error_set(LIN_ERR_IO_READ, "%s: read error", path);
    return LIN_ERR_IO_READ;
  }

  /* finalize hash */
  unsigned char md_value[EVP_MAX_MD_SIZE];
  unsigned int md_len = 0;
  EVP_DigestFinal_ex(md_ctx, md_value, &md_len);
  EVP_MD_CTX_free(md_ctx);
  fclose(fp);

  /* convert to hex string */
  for (unsigned int i = 0; i < md_len; i++)
    sprintf(stats->sha1 + i * 2, "%02x", md_value[i]);
  stats->sha1[md_len * 2] = '\0';

  stats->line_count = lines;
  stats->file_size  = (uint64_t)st.st_size;
  stats->exists     = true;
  stats->is_binary  = binary;

  return LIN_OK;
}
