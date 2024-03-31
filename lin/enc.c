#include "enc.h"

#include <string.h>
#include <openssl/evp.h>

int lin_hex_encode(int destination[], char value[], int value_length)
{
  int i, key = 9;
  for (i = 0; i < value_length; i++) {
    destination[i] = key ^ value[i];
  }
  return i;
}

int lin_hex_decode(char destination[], int value[], int value_length) {
  int i, key = 9;
  for (i = 0; i < value_length; i++) {
    destination[i] = key ^ value[i];
  }
  return i;
}

int lin_hash_sha1sum(char destination[], const char *value) {
  EVP_MD_CTX *md_ctx = NULL;
  const EVP_MD *md = NULL;
  unsigned char md_value[EVP_MAX_MD_SIZE];
  unsigned int md_len = 0, hash_len = 0;

  // init sha1sum
  md = EVP_get_digestbyname("SHA1");
  md_ctx = EVP_MD_CTX_create();

  // generate hash
  EVP_DigestInit_ex(md_ctx, md, NULL);
  EVP_DigestUpdate(md_ctx, value, strlen(value));
  EVP_DigestFinal_ex(md_ctx, md_value, &md_len);
  EVP_MD_CTX_destroy(md_ctx);

  // convert to hexadecimal and return
  for (unsigned int i = 0; i < md_len; i++) {
    hash_len += sprintf(destination + hash_len, "%02x", md_value[i]);
  }
  destination[hash_len] = '\0';

  return hash_len;
}
