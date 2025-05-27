#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

typedef struct map_t map_t;

map_t *map_alloc(size_t element_size);
void map_free(map_t *map);

bool map_insert(map_t *map, size_t key);
bool map_delete(map_t *map, size_t key);
bool map_search(map_t *map, size_t key);

#endif