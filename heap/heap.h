#ifndef HEAP_H
#define HEAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct heap_t heap_t;

heap_t *heap_alloc(size_t element_size, int (*compare)(const void*, const void*));
void heap_free(heap_t *heap);

void heapify(heap_t *heap, void *array, size_t element_count);
void heap_push(heap_t *heap, void *push_source);
void heap_pop(heap_t *heap, void *pop_destination);
void heap_replace(heap_t *heap, void *replace_source);
void heap_peek(heap_t *heap, void *peek_destination);

size_t heap_size(heap_t *heap);

void heap_print(heap_t *heap, void print_element(const void*));

#endif