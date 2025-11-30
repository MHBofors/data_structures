#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct darray_t darray_t;

darray_t *array_alloc(size_t element_size);
void array_free(darray_t *array);

bool array_insert(darray_t *array, void *insert_source, size_t index,
                  size_t count);
bool array_delete(darray_t *array, size_t index, size_t count);
bool array_replace(darray_t *array, void *replace_source, size_t replace_index,
                   size_t replace_count);
bool array_peek(darray_t *array, void *peek_destination, size_t index);

size_t array_size(darray_t *array);

void array_print(darray_t *array, void print_element(const void *));

#endif