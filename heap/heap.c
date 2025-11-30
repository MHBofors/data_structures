#include "heap.h"

#define HEAP_INIT_SIZE 32
#define HEAP_PARENT(i) ((i - 1) >> 1)
#define HEAP_CHILD_LEFT(i) (((i) << 1) + 1)
#define HEAP_CHILD_RIGHT(i) (((i) << 1) + 2)

struct heap_t {
	size_t element_size;
	size_t element_count;
	size_t heap_size;
	void *data;
	int (*compare)(const void *, const void *);
};

heap_t *heap_alloc(size_t element_size,
                   int (*compare)(const void *, const void *)) {
	heap_t *heap = malloc(sizeof(heap_t));
	void *data = malloc(HEAP_INIT_SIZE * element_size);

	if (heap != NULL && data != NULL && compare != NULL) {
		*heap = (heap_t){.element_size = element_size,
		                 .element_count = 0,
		                 .heap_size = HEAP_INIT_SIZE,
		                 .data = data,
		                 .compare = compare};

		return heap;
	} else {
		free(heap);
		free(data);
		return NULL;
	}
}

void heap_free(heap_t *heap) {
	free(heap->data);
	free(heap);
}

static void *heap_resize(heap_t *heap, size_t heap_size) {
	void *new_data = realloc(heap->data, heap->element_size * heap_size);

	if (new_data != NULL) {
		return heap->data = new_data;
	} else {
		return NULL;
	}
}

static void *heap_get_element(heap_t *heap, size_t index) {
	if (index < 0 || heap->element_count <= index) {
		return NULL;
	} else {
		return (char *)heap->data + heap->element_size * index;
	}
}

static void memswap(void *a, void *b, size_t size) {
	char temp, *A = a, *B = b;

	while (size-- > 0) {
		temp = *A;
		*A++ = *B;
		*B++ = temp;
	}
}

static void sift_up(heap_t *heap, size_t end) {
	char *data = heap->data;

	while (end > 0) {
		size_t root = HEAP_PARENT(end);

		if (heap->compare(data + heap->element_size * root,
		                  data + heap->element_size * end)) {
			memswap(data + heap->element_size * root,
			        data + heap->element_size * end, heap->element_size);
		} else {
			return;
		}
	}
}

static void sift_down(heap_t *heap, size_t start, size_t end) {
	size_t child, root = start;
	char *data = heap->data;

	while ((child = HEAP_CHILD_LEFT(root)) < end) {
		if (child + 1 < end &&
		    heap->compare(data + heap->element_size * child,
		                  data + heap->element_size * (child + 1))) {
			child++;
		}

		if (heap->compare(data + heap->element_size * root,
		                  data + heap->element_size * child)) {
			memswap(data + heap->element_size * root,
			        data + heap->element_size * child, heap->element_size);
			root = child;
		} else {
			break;
		}
	}
}

static size_t grow(size_t n) {
	n--;
	for (size_t i = 0; i < sizeof(size_t); i++) {
		n |= n >> (1 << i);
	}

	return (++n) << 1;
}

bool heapify(heap_t *heap, void *array, size_t element_count) {
	if (heap->heap_size <= heap->element_count + element_count) {
		size_t size = grow(heap->element_count + element_count);

		if (heap_resize(heap, size) == NULL) {
			return false;
		} else {
			heap->heap_size = size;
		}
	}

	void *leaf = (char *)heap->data + heap->element_size * heap->element_count;
	memcpy(leaf, array, heap->element_size * element_count);
	heap->element_count++;

	for (size_t start = heap->element_count >> 1; 0 <= start; start--) {
		sift_down(heap, start, heap->element_count - 1);
	}

	return true;
}

bool heap_push(heap_t *heap, void *push_source) {
	if (heap->heap_size <= heap->element_count) {
		size_t size = grow(heap->heap_size);

		if (heap_resize(heap, size) == NULL) {
			return false;
		} else {
			heap->heap_size = size;
		}
	}

	memcpy((char *)heap->data + heap->element_size * heap->element_count,
	       push_source, heap->element_size);
	heap->element_count++;

	for (size_t end = 0; end < heap->element_count; end++) {
		sift_up(heap, end);
	}

	return true;
}

bool heap_pop(heap_t *heap, void *pop_destination) {
	if (0 < heap->element_count) {
		memcpy(pop_destination, heap->data, heap->element_size);
		memcpy(heap->data,
		       (char *)heap->data +
		           heap->element_size * (heap->element_count - 1),
		       heap->element_size);
		heap->element_count--;
		sift_down(heap, 0, heap->element_count);
		return true;
	} else {
		return false;
	}
}

bool heap_replace(heap_t *heap, void *replace_source) {
	if (0 < heap->element_count) {
		memswap(heap->data, replace_source, heap->element_size);
		sift_down(heap, 0, heap->element_count);
		return true;
	} else {
		return false;
	}
}

bool heap_peek(heap_t *heap, void *peek_destination) {
	if (0 < heap->element_count) {
		memcpy(peek_destination, heap->data, heap->element_size);
		return true;
	} else {
		return false;
	}
}

size_t heap_size(heap_t *heap) { return heap->element_count; }
