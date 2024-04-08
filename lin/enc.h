#ifndef LIN_HASH_H_
#define LIN_HASH_H_

#include <stdio.h>


// These functions are used to encode and decode a string
// using a simple XOR operation. Hereby, the safety is not
// guaranteed, only use them to encode and decode simple
// strings that do not contain sensitive information
int lin_hex_encode(int destination[], char value[], int value_length);

int lin_hex_decode(char destination[], int value[], int value_length);

int lin_hash_sha1sum(char destination[], const char *value);

int lin_hash_sha1sum_from_file(char destination[64], FILE *fptr);

#endif // LIN_HASH_H_