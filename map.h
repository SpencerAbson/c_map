#ifndef MAP_H_
#define MAP_H_

#include <stdint.h>

typedef enum map_result
{
hashmap_result_success,
hashmap_result_no_memory,
hashmap_result_bad_key,
hashmap_result_bad_argument,
hashmap_result_bad_value
}hashmap_result;


typedef struct map_item
{
    struct map_item *next;

    void *key;
    size_t key_len;

    uint32_t hash;
    uintptr_t value;

}map_item_t;


typedef struct hashmap
{
    map_item_t **buckets;

    uint32_t entries;
    uint32_t capacity;
}hashmap_t;


hashmap_result hashmap_init(hashmap_t *map, uint32_t initial_size);
hashmap_t *hashmap_create(uint32_t initial_size);
hashmap_result hashmap_set(hashmap_t **map, void  *key, size_t key_len, uintptr_t const value);
hashmap_result hashmap_set_u32(hashmap_t **map, uint32_t key, uintptr_t const value);
hashmap_result hashmap_get(hashmap_t *map, void *key, size_t key_len, uintptr_t *result);
hashmap_result hashmap_get_u32(hashmap_t *map, uint32_t key, uintptr_t *result);
hashmap_result hashmap_destroy(hashmap_t *map);


#endif // MAP_H_
