#include "heap.h"

#define HEAP_INIT_SIZE 32

static void memswap(void *a, void *b, size_t size) {
	char temp, *A = a, *B = b;

	while (size-- > 0) {
		temp = *A;
		*A++ = *B;
		*B++ = temp;
	}
}



void sift_down_r(size_t i, void *base, size_t count, size_t size,
             int (*compare)(const void *, const void *)) {
	size_t i_min = i, left = HEAP_CHILD_LEFT(i), right = HEAP_CHILD_RIGHT(i);

	if(left < count && compare((char *)base + left*size, (char *)base + i_min*size) == -1) {
		i_min = left;
	}

	if(right < count && compare((char *)base + right*size, (char *)base + i_min*size) == -1) {
		i_min = right;
	}

	if(i_min != i) {
		memswap((char *)base + i_min*size, (char *)base + i*size, sizeof(size_t));
		sift_down(i_min, base, count, size, compare);
	}
}



void sift_down(size_t i, void *base, size_t count, size_t size,
             int (*compare)(const void *, const void *)) {
	while(i < count) {
		size_t i_min = i, left = HEAP_CHILD_LEFT(i), right = HEAP_CHILD_RIGHT(i);
		if(left < count && compare((char *)base + left*size, (char *)base + i_min*size) == -1) {
			i_min = left;
		}

		if(right < count && compare((char *)base + right*size, (char *)base + i_min*size) == -1) {
			i_min = right;
		}

		if(i_min != i) {
			memswap((char *)base + i_min*size, (char *)base + i*size, sizeof(size_t));
			i = i_min;
			continue;
		} else {
			break;
		}
	}

}



void sift_up(size_t i, void *base, size_t size,
           int (*compare)(const void *, const void *)) {
	while(0 < i) {
		size_t parent = HEAP_PARENT(i);

		if(compare((char *)base + i*size, (char *)base + parent*size) == -1) {
			memswap((char *)base + i*size, (char *)base + parent*size, size);
			i = parent;
		} else {
			break;
		}
	}
}



void sift_up_r(size_t i, void *base, size_t size,
           int (*compare)(const void *, const void *)) {
	size_t parent = HEAP_PARENT(i);

	if(i > 0 && compare((char *)base + i*size, (char *)base + parent*size) == -1) {
		memswap((char *)base + i*size, (char *)base + parent*size, size);
		sift_up(parent, base, size, compare);
	}
}

void heap_push(void *src, void *base, size_t count, size_t size, 
             int (*compare)(const void *, const void *)) {
	memcpy((char *)base + count*size, src, size);
	sift_up(count, base, size, compare);
}

void heap_pop(void *dst, void *base, size_t count, size_t size, 
             int (*compare)(const void *, const void *)) {
	if(0 < count) {
		if(dst != NULL) {
			memcpy(dst, base, size);
		}

		memmove(base, (char *)base + (count - 1)*size, size);
		sift_down(0, base, count, size, compare);
	}
}



void heap_update(void *src, size_t i, void *base, size_t count, size_t size, 
             int (*compare)(const void *, const void *)) {
	if(compare(src, (char *)base + i*size) == -1) {
		/* Decrease key */
		memmove((char *)base + i*size, src, size);
		sift_up(i, base, size, compare);
	} else {
		/* Increase key */
		memmove((char *)base + i*size, src, size);
		sift_down(i, base, count, size, compare);
	}
}



void heap_delete(size_t i, void *base, size_t count, size_t size, 
             int (*compare)(const void *, const void *)) {
	
	if(compare((char *)base + (count - 1)*size, (char *)base + i*size) == -1) {
		memmove((char *)base + i*size, (char *)base + (count - 1)*size, size);
		sift_up(i, base, size, compare);
	} else {
		memmove((char *)base + i*size, (char *)base + (count - 1)*size, size);
		sift_down(i, base, count - 1, size, compare);
	}
}