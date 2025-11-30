
#include "hash.h"

size_t strnhash(const void *key, size_t key_size, const void *seed) {
    const unsigned char *str = key;
    const size_t *seed_buf = seed;
    size_t hash = 0;
    char h[sizeof(size_t)] = {0};
    for(size_t i = 0; i < key_size && str[i] != '\0'; i++) {
        hash ^= seed_buf[str[i] % 128U];
    }
    
    return hash;
}

size_t zhash(const void *key, size_t key_size, const void *seed) {
    const unsigned char *str = key;
    const size_t *seed_buf = seed;
    size_t hash = 0;
    char h[sizeof(size_t)] = {0};
    for(size_t i = 0; i < key_size; i++) {
        hash ^= seed_buf[str[i] % 128U];
    }
    
    return hash;
}

