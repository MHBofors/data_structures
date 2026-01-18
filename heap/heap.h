#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_PARENT(i) ((i - 1) >> 1)
#define HEAP_CHILD_LEFT(i) (((i) << 1) + 1)
#define HEAP_CHILD_RIGHT(i) (((i) << 1) + 2)

void sift_down(size_t i, void *base, size_t count, size_t size, 
             int (*compare)(const void *, const void *));
void sift_up(size_t i, void *base, size_t size,
           int (*compare)(const void *, const void *));


void heap_push(void *src, void *base, size_t count, size_t size, 
             int (*compare)(const void *, const void *));
void heap_pop(void *dst, void *base, size_t count, size_t size, 
             int (*compare)(const void *, const void *));
void heap_update(void *src, size_t i, void *base, size_t count, size_t size, 
             int (*compare)(const void *, const void *));
             
#endif