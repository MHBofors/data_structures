#include "dynamic_array.h"

#define ARRAY_INIT_SIZE 32

struct darray_t {
    size_t element_size;
    size_t element_count;
    size_t array_size;
    void *data;
};

darray_t *array_alloc(size_t element_size) {
    darray_t *array = NULL;
    void *data = NULL;

    if((array = malloc(sizeof(darray_t))) && (data = malloc(ARRAY_INIT_SIZE * element_size))) {
        *array = (darray_t){
            .element_size = element_size,
            .element_count = 0,
            .array_size = ARRAY_INIT_SIZE,
            .data = data
        };

        return array;
    } else {
        free(array);
        free(data);
        return NULL;
    }
}

void array_free(darray_t *array) {
    if(array) {
        free(array->data);
        free(array);
    }
}

static void *array_resize(darray_t *array, size_t array_size) {
    void *new_data = realloc(array->data, array->element_size * array_size);

    if(new_data != NULL) {
        return array->data = new_data;
    } else {
        return NULL;
    }
}



static void *array_get_element(darray_t *array, size_t index) {
    if(array->element_count <= index) {
        return NULL;
    } else {
        return (char *)array->data + array->element_size*index;
    }
}

static void memswap(void *a, void *b, size_t size) {
    char temp, *A = a, *B = b;

    while(size-- > 0) {
        temp = *A;
        *A++ = *B;
        *B++ = temp;
    }
}



static size_t grow(size_t n) {
    n--;
    for(size_t i = 0; i < sizeof(size_t); i++) {
        n |= n >> (1 << i);
    }

    return (++n) << 1;
}

bool array_append(darray_t *array, void *append_source, size_t append_count) {
    if(array->array_size + append_count <= array->element_count) {
        size_t size = grow(array->array_size);

        if(array_resize(array, size) == NULL) {
            return false;
        } else {
            array->array_size = size;
        }
    }
    
    memcpy((char *)array->data + array->element_size*array->element_count, append_source, array->element_size*append_count);
    array->element_count += append_count;


    return true;
}

bool array_insert(darray_t *array, void *insert_source, size_t insert_index, size_t insert_count) {
    if(array->element_count < insert_index || insert_count == 0) {
        return false;
    }

    if(array->array_size + insert_count <= array->element_count) {
        size_t size = grow(array->array_size + insert_count);

        if(array_resize(array, size) == NULL) {
            return false;
        } else {
            array->array_size = size;
        }
    }
    
    memmove((char *)array->data + array->element_size*(insert_index + insert_count), (char *)array->data + array->element_size*insert_index, array->element_size*(array->element_count - insert_index));
    memcpy((char *)array->data + array->element_size*insert_index, insert_source, array->element_size*insert_count);
    array->element_count += insert_count;

    return true;
}

bool array_delete(darray_t *array, size_t delete_index, size_t delete_count) {
    if(array->element_count < delete_index + delete_count || delete_count == 0) {
        return false;
    } else {
        memmove((char *)array->data + array->element_size*delete_index, (char *)array->data + array->element_size*(delete_index + delete_count), array->element_size*(array->element_count - (delete_index + delete_count)));
        array->element_count -= delete_count;
        return true;
    }
}

bool array_replace(darray_t *array, void *replace_source, size_t replace_index, size_t replace_count) {
    if(array->element_count < replace_index + replace_count || replace_count == 0) {
        return false;
    } else {
        memcpy((char *)array->data + array->element_size*replace_index, replace_source, array->element_size*replace_count);
        return true;
    }
}

bool array_peek(darray_t *array, void *peek_destination, size_t index) {
    if(array->element_count < index) {
        return false;
    } else {
        memcpy(peek_destination, (char *)array->data + array->element_size*index, array->element_size);
        return true;
    }
}



size_t array_size(darray_t *array) {
    return array->element_count;
}

void array_print(darray_t *array, void print_element(const void*)) {
    for(size_t i = 0; (1 << i) - 1 < array->element_count; i++) {
        print_element((char *)array->data + i*array->element_size);
        printf("\n");
    }
}