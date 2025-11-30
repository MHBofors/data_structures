#ifndef HASH_H
#define HASH_H
#include <stdlib.h>

#define SEED_SIZE (1UL << 10)

typedef size_t (*hash_t)(const void *key, size_t key_size, const void *seed);

size_t strnhash(const void *key, size_t key_size, const void *seed);
size_t zhash(const void *key, size_t key_size, const void *seed);

#endif