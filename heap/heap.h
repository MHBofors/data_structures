#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct heap_t heap_t;

heap_t *heap_alloc(size_t element_size,
                   int (*compare)(const void *, const void *));
void heap_free(heap_t *heap);

bool heapify(heap_t *heap, void *array, size_t element_count);
bool heap_push(heap_t *heap, void *push_source);
bool heap_pop(heap_t *heap, void *pop_destination);
bool heap_replace(heap_t *heap, void *replace_source);
bool heap_peek(heap_t *heap, void *peek_destination);

size_t heap_size(heap_t *heap);

#endif