#include "hash_table.h"
#include <time.h>

#define STASH_SIZE 8
#define MAP_INIT_SIZE 8
#define ALPHA 4
#define EPSILON 1.5f

struct map_t {
    size_t table_size;
    size_t key_count;
    size_t insertion_count;
    size_t h_0[8][256], h_1[8][256];
    size_t stash[STASH_SIZE];
    size_t *table;/* 1 << table_size + 1 */
};



void generate_hash(size_t m, size_t n, size_t T[m][n]) {
    arc4random_buf(T, sizeof(size_t)*m*n);
    for(size_t i = 0; i < m; i++) {
        for(size_t j = 0; j < n; j++) {

        }
    }
}

size_t hash(size_t key, size_t T[8][256]) {
    size_t H = 0;
    for(size_t i = 0; i < sizeof(size_t); i++) {
        H ^= T[i][(unsigned char)(key >> 8*i)]; 
    }

    return H;
}

map_t *map_alloc(size_t element_size) {
    map_t *map = malloc(sizeof(map_t));
    size_t *table = calloc(1 << MAP_INIT_SIZE, sizeof(size_t));

    if(map != NULL && map != NULL) {
        *map = (map_t){
            .table_size = MAP_INIT_SIZE,
            .key_count = 0,
            .stash = {0},
            .insertion_count = 0,
            .table = table
        };
        
        generate_hash(8, 256, map->h_0);
        generate_hash(8, 256, map->h_1);
        return map;
    } else {
        free(map);
        free(table);
        return NULL;
    }
}

void map_free(map_t *map) {
    free(map->table);
    free(map);
}



static size_t max_iterations(size_t n) {
    size_t m;
    for(m = 8*sizeof(size_t); n; n >>= 1, --m);
    return m;
}

static void swap(size_t *a, size_t *b) {
    size_t temp = *a;
    *a = *b;
    *b = temp;
}

static inline size_t *map_resize(map_t *map, size_t map_size) {
    size_t *new_table = realloc(map->table, sizeof(size_t)*(1 << (map_size)));
    
    if(new_table != NULL) {
        map->table = new_table;

        for(size_t i = 1 << (map_size - 1); i < (1 << map_size); i++) {
            map->table[i] = 0;
        }
        return map->table;
    } else {
        return NULL;
    }
}

static size_t map_cuckoo(map_t *map, size_t key, size_t N) {
    if(key) {
        for(size_t i = 0; i < N; i++) {
            size_t h_0 = (hash(key, map->h_0) & (1 << (map->table_size)) - 1) & ~1;
            swap(&key, &map->table[h_0]);
            if(key == 0) return 0;
    
            size_t h_1 = (hash(key, map->h_1) & (1 << (map->table_size)) - 1) | 1;
            swap(&key, &map->table[h_1]);
            if(key == 0) return 0;
        }
    
        for(size_t i = 0; i < STASH_SIZE; i++) {
            swap(&key, &map->stash[i]);
            if(key == 0) return 0;
        }
    }

    return key;
}

static void map_rehash(map_t *map) {
    bool rehashed;
    size_t max_iter = ALPHA*map->table_size;
    size_t key = 0;

    do {
        rehashed = true;
        generate_hash(8, 256, map->h_0);
        generate_hash(8, 256, map->h_1);

        if(key = map_cuckoo(map, key, max_iter)) {
            rehashed = false;
            continue;
        }

        for(size_t i = 0; i < 1 << (map->table_size) && rehashed; i++) {
            key = map->table[i];
            map->table[i] = 0;
            if(key = map_cuckoo(map, key, max_iter)) {
                rehashed = false;
            }
        }


    } while(!rehashed);

    map->insertion_count = 0;
}

bool map_insert(map_t *map, size_t key) {
    if(!key) {
        return false; // 0 is not a valid key
    } else if(map_search(map, key)) {
        return true;
    } else {
        if(1UL << map->table_size <= (3*map->key_count)) {
            size_t new_size = map->table_size + 2;
            
            if(!map_resize(map, new_size)) {
                return false;
            } else {
                map->table_size = new_size;
                map_rehash(map);
            }
        }
        
        if((1<<map->table_size) < map->insertion_count) {
            map_rehash(map);
        }

        size_t max_iter = ALPHA*map->table_size;
        for(size_t i = 0; i < STASH_SIZE; i++) {
            if(map->stash[i]) {
                map->stash[i] = map_cuckoo(map, map->stash[i], max_iter);
            }
        }
        while(key = map_cuckoo(map, key, max_iter)) {
            map_rehash(map);
        }

        map->key_count++;
        map->insertion_count++;
        return true;
    }
}

bool map_delete(map_t *map, size_t key) {
    size_t *key_0;
    key_0 = &map->table[(hash(key, map->h_0) & (1 << (map->table_size)) - 1) & ~1];
    if(*key_0 == key) {
        *key_0 = 0;
        map->key_count--;
        return true;
    }
    key_0 = &map->table[(hash(key, map->h_1) & (1 << (map->table_size)) - 1) | 1];
    if(*key_0 == key) {
        *key_0 = 0;
        map->key_count--;
        return true;
    }

    for(size_t i = 0; i < STASH_SIZE; i++) {
        if(map->table[i] == key) {
            map->table[i] = 0;
            map->key_count--;
            return true;
        }
    }

    return false;
}

bool map_search(map_t *map, size_t key) {
    bool key_found = 0;

    if(key == map->table[(hash(key, map->h_0) & (1 << (map->table_size)) - 1) & ~1]) return true;
    if(key == map->table[(hash(key, map->h_1) & (1 << (map->table_size)) - 1) | 1]) return true;

    //key_found |= (key == map->table[(hash(key, map->h_0) & (1 << (map->table_size)) - 1) & ~1]);
    //key_found |= (key == map->table[(hash(key, map->h_1) & (1 << (map->table_size)) - 1) | 1]);

    for(size_t i = 0; i < STASH_SIZE; i++) {
        if(key == map->stash[i]) return true;
    }

    return false;
}