#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "hash.h"

typedef struct map_t map_t;
typedef size_t (*write_t)(void *restrict, const void *restrict, size_t);
typedef int (*compare_t)(const void*, const void*);

map_t *map_alloc(size_t key_size, size_t value_size, size_t alignment);
map_t *map_alloc_strn(size_t key_size, size_t value_size);
void map_free(map_t *map);

void map_set_hash(map_t *map, size_t (*hash)(const void*, size_t, const void*));
void map_set_is_equal(map_t *map, int (*is_equal)(const void*, const void*, size_t));
void map_set_key_write(map_t *map, void (*key_write)(void *restrict, const void *restrict, size_t));
void map_set_value_write(map_t *map, void (*value_write)(void *restrict, const void *restrict, size_t));

bool map_insert(map_t *map, const void *key, const void *value);
bool map_delete(map_t *map, const void *key);
bool map_search(map_t *map, const void *key);
void *map_lookup(map_t *map, const void *key, void *lookup_dst);

void *map_for(map_t *map, const void *key, void (*f)(void*));
void *map_for_each(map_t *map, void (*f)(void*));
size_t map_count(map_t *map);

static void map_rehash(map_t *map);

static void *map_cuckoo(map_t *map, void *item, size_t max_iter);

#endif