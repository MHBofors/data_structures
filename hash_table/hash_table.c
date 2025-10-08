#include "hash_table.h"

#define BUCKET_SIZE 8UL
#define STASH_SIZE 8UL
#define MAP_INIT_SIZE 4UL
#define ALPHA 4UL
#define EPSILON 1.5f
#define BUCKET_ALIGNMENT 64UL

#define table_construct(KEY_TYPE, VALUE_TYPE) map_alloc(sizeof(KEY_TYPE), sizeof(VALUE_TYPE), _Alignof(KEY_TYPE) > _Alignof(VALUE_TYPE) ? _Alignof(KEY_TYPE) : _Alignof(VALUE_TYPE))

struct map_t {
    size_t entry_count;
    size_t insertion_count;
    size_t table_size;

    size_t key_size;
    size_t value_size;
    size_t alignment;

    size_t (*hash)(const void *restrict, size_t, const void *restrict);
    int (*is_equal)(const void*, const void*, size_t);

    void (*key_write)(void *restrict, const void *restrict, size_t);
    void (*value_write)(void *restrict, const void *restrict, size_t);

    size_t seed_size;
    void *seed_0, *seed_1;

    void *(*table_0)[BUCKET_SIZE], *(*table_1)[BUCKET_SIZE];
    void *stash[STASH_SIZE];
};

void *ptr_align_up(void *ptr, size_t alignment) {
    if(!alignment || alignment & (alignment - 1)) {
        return NULL; 
    }
    return (void *)(((uintptr_t)ptr + alignment - 1) & ~(alignment - 1));
}

size_t entry_size(size_t key_size, size_t value_size, size_t alignment) {
    size_t offset = (key_size + alignment - 1) & ~(alignment - 1);
    offset += (value_size + alignment - 1) & ~(alignment - 1);
    return offset;
}

void *alloc_entry(size_t key_size, size_t value_size, size_t alignment) {
    return malloc(entry_size(key_size, value_size, alignment));
}

void generate_seed(void *T, size_t n) {
    arc4random_buf(T, n);
}

size_t map_count(map_t *map) {
    return map->entry_count;
}

void map_free(map_t *map) {
    for(size_t i = 0; i < (1 << map->table_size); i++) {
        for(size_t j = 0; j < BUCKET_SIZE; j++) {
            free(map->table_0[i][j]);
            free(map->table_1[i][j]);
        }
    }
    for(size_t i = 0; i < STASH_SIZE; i++) {
        free(map->stash[i]);
    }

    free(map->table_0);
    free(map->table_1);

    free(map->seed_0);
    free(map->seed_1);

    free(map);
}



map_t *map_alloc(size_t key_size, size_t value_size, size_t alignment) {
    if(key_size == 0 || value_size == 0 || (alignment & (alignment - 1))) {
        return NULL; // Invalid key or value size
    }

    map_t *map = malloc(sizeof(map_t));
    void *seed_0 = aligned_alloc(BUCKET_ALIGNMENT, SEED_SIZE);
    void *seed_1 = aligned_alloc(BUCKET_ALIGNMENT, SEED_SIZE);
    void *table_0 = aligned_alloc(BUCKET_ALIGNMENT, sizeof(void*)*BUCKET_SIZE*(1 << MAP_INIT_SIZE));
    void *table_1 = aligned_alloc(BUCKET_ALIGNMENT, sizeof(void*)*BUCKET_SIZE*(1 << MAP_INIT_SIZE));
    void *stash = aligned_alloc(BUCKET_ALIGNMENT, sizeof(void*)*STASH_SIZE);

    if(map != NULL && map != NULL && seed_0 != NULL && seed_1 != NULL && table_0 != NULL && table_1 != NULL && stash != NULL) {
        *map = (map_t){
            .entry_count = 0,
            .insertion_count = 0,
            .table_size = MAP_INIT_SIZE,

            .key_size = key_size,
            .value_size = value_size,
            .alignment = alignment,

            .hash = zhash,
            .is_equal = memcmp,

            .key_write = (void (*)(void *restrict, const void *restrict, size_t))memcpy,
            .value_write = (void (*)(void *restrict, const void *restrict, size_t))memcpy,
            
            .seed_size = SEED_SIZE,
            .seed_0 = seed_0,
            .seed_1 = seed_1,

            .table_0 = table_0,
            .table_1 = table_1,
            .stash = stash
        };
        
        generate_seed(seed_0, SEED_SIZE);
        generate_seed(seed_1, SEED_SIZE);

        for(size_t i = 0; i < (1 << map->table_size); i++) {
            for(size_t j = 0; j < BUCKET_SIZE; j++) {
                map->table_0[i][j] = NULL;
                map->table_1[i][j] = NULL;
            }
        }

        for(size_t i = 0; i < STASH_SIZE; i++) {
            map->stash[i] = NULL;
        }
        return map;
    } else {
        free(map->seed_0);
        free(map->seed_1);
        free(map->table_0);
        free(map->table_1);
        free(map->stash);
        free(map);
        return NULL;
    }
}

map_t *map_alloc_strn(size_t key_size, size_t value_size) {
    map_t *map = map_alloc(key_size, value_size, _Alignof(char));
    map_set_hash(map, strnhash);
    map_set_is_equal(map, (int (*)(const void*, const void*, size_t))strncmp);
    map_set_key_write(map, (void (*)(void *restrict, const void *restrict, size_t))strncpy);
    map_set_value_write(map, (void (*)(void *restrict, const void *restrict, size_t))strncpy);
    return map;
}

void map_set_hash(map_t *map, size_t (*hash)(const void*, size_t, const void*)) {
    map->hash = hash;
}
void map_set_is_equal(map_t *map, int (*is_equal)(const void*, const void*, size_t)) {
    map->is_equal = is_equal;
}
void map_set_key_write(map_t *map, void (*key_write)(void *restrict, const void *restrict, size_t)) {
    map->key_write = key_write;
}
void map_set_value_write(map_t *map, void (*value_write)(void *restrict, const void *restrict, size_t)) {
    map->value_write = value_write;
}

static inline bool map_resize(map_t *map, size_t new_size) {
    void *new_table_0 = aligned_alloc(BUCKET_ALIGNMENT, sizeof(void*)*BUCKET_SIZE*(1 << (new_size)));
    void *new_table_1 = aligned_alloc(BUCKET_ALIGNMENT, sizeof(void*)*BUCKET_SIZE*(1 << (new_size)));
    
    if(new_table_0 != NULL && new_table_1 != NULL) {
        memcpy(new_table_0, map->table_0, sizeof(void*)*BUCKET_SIZE*(1 << (map->table_size)));
        memcpy(new_table_1, map->table_1, sizeof(void*)*BUCKET_SIZE*(1 << (map->table_size)));
        
        free(map->table_0);
        free(map->table_1);
        
        map->table_0 = new_table_0;
        map->table_1 = new_table_1;

        for(size_t i = (1 << map->table_size); i < (1 << new_size); i++) {
            for(size_t j = 0; j < BUCKET_SIZE; j++) {
                map->table_0[i][j] = NULL;
                map->table_1[i][j] = NULL;
            }
        }

        map->table_size = new_size;
        return true;
    } else {
        free(new_table_0);
        free(new_table_1);
        return false;
    }
}

static bool map_should_resize(map_t *map) {
    return ((1UL << (map->table_size + 1))*BUCKET_SIZE <= (3*map->entry_count));
}

static void *map_probe_bucket(map_t *map, void *bucket[BUCKET_SIZE], const void *restrict key) {
    for(size_t i = 0; i < BUCKET_SIZE; i++) {
        if(bucket[i] && map->is_equal(key, bucket[i], map->key_size) == 0) {
            return bucket[i];
        }
    }

    return NULL;
}

static void *map_probe_stash(map_t *map, const void *restrict key) {
    for(size_t i = 0; i < STASH_SIZE; i++) {
        if(map->stash[i] && map->is_equal(key, map->stash[i], map->key_size) == 0) {
            return map->stash[i];
        }
    }

    return NULL;
}

static void *map_cuckoo_bucket(void *item, void *bucket[BUCKET_SIZE]) {
    if(bucket[BUCKET_SIZE - 1]) {
        void *temp = bucket[BUCKET_SIZE - 1];
        memmove(bucket + 1, bucket, (BUCKET_SIZE - 1)*sizeof(void*));
        bucket[0] = item;
        return temp;
    } else {
        for(size_t i = 0; i < BUCKET_SIZE; i++) {
            if(!bucket[i]) {
                bucket[i] = item;
                return NULL;
            }
        }
    }

    return item;
}

static inline void *map_cuckoo(map_t *map, void *item, size_t max_iter) {
    if(item) {
        for(size_t i = 0; i < max_iter; i++) {
            size_t h_0 = (map->hash(item, map->key_size, map->seed_0) & ((1 << (map->table_size)) - 1));
            item = map_cuckoo_bucket(item, map->table_0[h_0]);
            if(!item) return NULL;
    
            size_t h_1 = (map->hash(item, map->key_size, map->seed_1) & ((1 << (map->table_size)) - 1));
            item = map_cuckoo_bucket(item, map->table_1[h_1]);
            if(!item) return NULL;
        }

        for(size_t i = 0; i < STASH_SIZE; i++) {
            if(!map->stash[i]) {
                map->stash[i] = item;
                return NULL;
            }
        }

        return item;
    } else {
        return NULL;
    }
}

static void *map_probe(map_t *map, const void *key) {
    size_t h_0 = (map->hash(key, map->key_size, map->seed_0) & ((1 << (map->table_size)) - 1));
    void *item = map_probe_bucket(map, map->table_0[h_0], key);
    if(item) return item;

    size_t h_1 = (map->hash(key, map->key_size, map->seed_1) & ((1 << (map->table_size)) - 1));
    item = map_probe_bucket(map, map->table_1[h_1], key);
    if(item) return item;

    item = map_probe_stash(map, key);
    if(item) return item;

    return NULL;
}

void *map_lookup(map_t *map, const void *key, void *lookup_dst) {
    void *item = map_probe(map, key);

    if(item) {
        void *value = ptr_align_up((char*)item + map->key_size, map->alignment);

        if(lookup_dst) {
            map->value_write(lookup_dst, value, map->value_size);
        }
        
        return value;
    } else {
        return NULL;
    }
}

bool map_insert(map_t *map, const void *key, const void *value) {
    if(!key || !value) {
        return false; // 0 is not a valid key or value
    } else if(map_probe(map, key)) {
        return true;
    } else {
        if(map_should_resize(map)) {
            size_t new_size = map->table_size + 1;

            if(!map_resize(map, new_size)) {
                return false;
            }

            map_rehash(map);
        }

        void *item = alloc_entry(map->key_size, map->value_size, map->alignment);
        if(!item) return false;

        map->key_write(item, key, map->key_size);
        void *value_ptr = ptr_align_up((char*)item + map->key_size, map->alignment);
        map->value_write(value_ptr, value, map->value_size);

        size_t max_iter = ALPHA*map->table_size;
        for(size_t i = 0; i < STASH_SIZE; i++) {
            if(map->stash[i]) {
                map->stash[i] = map_cuckoo(map, map->stash[i], max_iter);
            }
        }
        while(item = map_cuckoo(map, item, max_iter)) {
            map_rehash(map);
        }

        map->entry_count++;
        map->insertion_count++;
        return true;
    }
}

void map_rehash(map_t *map) {
    size_t max_iter = ALPHA*map->table_size;
    void *item = NULL;

    do {
        generate_seed(map->seed_0, SEED_SIZE);
        generate_seed(map->seed_1, SEED_SIZE);

        if(item = map_cuckoo(map, item, max_iter))  goto restart;

        for(size_t i = 0; i < 1 << (map->table_size); i++) {
            for(size_t j = 0; j < BUCKET_SIZE; j++) {
                item = map->table_0[i][j];
                map->table_0[i][j] = NULL;
                if(item = map_cuckoo(map, item, max_iter)) goto restart;
            }
            for(size_t j = 0; j < BUCKET_SIZE; j++) {
                item = map->table_1[i][j];
                map->table_1[i][j] = NULL;
                if(item = map_cuckoo(map, item, max_iter)) goto restart;
            }
        }

        restart:
    } while(item);

    map->insertion_count = 0;
}

static bool map_delete_bucket(map_t *map, void *bucket[BUCKET_SIZE], const void *restrict key) {
    for(size_t i = 0; i < BUCKET_SIZE; i++) {
        if(bucket[i] && map->is_equal(key, bucket[i], map->key_size) == 0) {
            free(bucket[i]);
            bucket[i] = NULL;
            return true;
        }
    }

    return false;
}

static bool map_delete_stash(map_t *map, const void *restrict key) {
    for(size_t i = 0; i < STASH_SIZE; i++) {
        if(map->stash[i] && map->is_equal(key, map->stash[i], map->key_size) == 0) {
            free(map->stash[i]);
            map->stash[i] = NULL;
            return true;
        }
    }

    return false;
}

bool map_delete(map_t *map, const void *key) {
    if(!key) {
        return false;
    }

    size_t h_0 = (map->hash(key, map->key_size, map->seed_0) & ((1 << (map->table_size)) - 1));
    if(map_delete_bucket(map, map->table_0[h_0], key)) {
        map->entry_count--;
        return true;
    }

    size_t h_1 = (map->hash(key, map->key_size, map->seed_1) & ((1 << (map->table_size)) - 1));
    if(map_delete_bucket(map, map->table_1[h_1], key)) {
        map->entry_count--;
        return true;
    }

    if(map_delete_stash(map, key)) {
        map->entry_count--;
        return true;
    }

    return false;
}