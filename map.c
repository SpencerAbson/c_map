#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "lookup3.h"
#include "map.h"

#define HASHMAP_INITIAL_CAPACITY 512
#define HASHMAP_GROW_FACTOR      3
#define HASHMAP_MAX_LOAD_FACTOR  1

#define u32_byte_string(val)                    \
    {val & 0xFF, (val << 8) & 0xFF, (val << 16) &  0xFF, (val << 24) & 0xFF}

extern void hashmap_ensure_seed();
extern uint32_t hashmap_seed;

/* remember to change all of the malloc stuff to si_malloc etc */
static map_item_t *_map_item_create(void *key, size_t key_len, uintptr_t const value, uint32_t hash)
{
    map_item_t *item = malloc(sizeof(map_item_t));
    if(!item) return NULL;

    item->key = key;
    item->key_len = key_len;

    item->hash = hash; // so that the item is uniquely identifiable within a bucket

    item->value = value;
    item->next = NULL;

    return item;
}


static inline void _add_to_bucket(map_item_t *bucket, map_item_t *item)
{
    map_item_t *prev = bucket;

    while(prev->next != NULL)
        prev = prev->next;

    prev->next = item;
}


static hashmap_result _hashmap_internal_set(hashmap_t *map, void *key, size_t key_len, uintptr_t const value)
{
    map_item_t *new_item;
    uint32_t hash, bucket_index;

    hash = hashlittle(key, key_len, hashmap_seed);
    bucket_index = hash % map->capacity;

    new_item = _map_item_create(key, key_len, value, hash);
    if(!new_item)
        return hashmap_result_no_memory;

    if(map->buckets[bucket_index] != 0)
        _add_to_bucket(map->buckets[bucket_index], new_item);
    else{
        // no, then the bucket is the item !
        map->buckets[bucket_index] = new_item;
    }

    map->entries++;
    return hashmap_result_success;
}


hashmap_result hashmap_init(hashmap_t *map, uint32_t initial_size)
{
    if(!map)
        return hashmap_result_bad_argument;

    map->capacity = initial_size;
    map->entries  = 0;
    map->buckets  = calloc(initial_size, sizeof(map_item_t*));

    hashmap_ensure_seed();

    return hashmap_result_success;
}


hashmap_t *hashmap_create(uint32_t initial_size)
{
    hashmap_t *map = malloc(sizeof(hashmap_t));
    if(!map)
        return NULL;

    if(hashmap_init(map, initial_size) != hashmap_result_success){
        free(map);
        return NULL;
    }

    return map;
}


static hashmap_t * _hashmap_grow(hashmap_t *map)
{
    hashmap_t *new_map;
    map_item_t *curr_item, *temp_item;

    new_map = hashmap_create(map->capacity * HASHMAP_GROW_FACTOR);
    if(!new_map) return NULL;

    for(uint32_t i = 0; i < map->capacity; i++)
    {
        if(map->buckets[i] != 0)
        {
            curr_item = map->buckets[i];
            do{
                temp_item = curr_item;

                if(_hashmap_internal_set(new_map, curr_item->key, curr_item->key_len, curr_item->value)
                   != hashmap_result_success)
                    return NULL;

                curr_item = curr_item->next;
                free(temp_item);
            }while(curr_item != NULL);
        }
    }

    free(map->buckets);
    free(map);

    return new_map;
}


hashmap_result hashmap_set(hashmap_t **map, void  *key, size_t key_len, uintptr_t const value)
{
    if((*map) == NULL)
        return hashmap_result_bad_argument;

    if(((*map)->entries + 1) >= ((*map)->capacity * HASHMAP_MAX_LOAD_FACTOR))
        *map = _hashmap_grow(*map);


    return _hashmap_internal_set((*map), key, key_len, value);
}


hashmap_result hashmap_set_u32(hashmap_t **map, uint32_t key, uintptr_t const value)
{
    char as_bytes[4] = u32_byte_string(key);
    return hashmap_set(map, as_bytes, 4, value);
}


static map_item_t * _find_entry(hashmap_t *map, void *key, size_t key_len)
{
    map_item_t *bucket_head;
    uint32_t hash = hashlittle(key, key_len, hashmap_seed);
    uint32_t bucket_index = hash % map->capacity;

    if(map->buckets[bucket_index] == 0)
        return NULL;

    bucket_head = map->buckets[bucket_index];
    while(bucket_head != NULL)
    {
        if(bucket_head->hash == hash)
            return bucket_head;
        bucket_head = bucket_head->next;
    }

    return NULL;
}


hashmap_result hashmap_get(hashmap_t *map, void *key, size_t key_len, uintptr_t *result)
{
    map_item_t *item;
    if(!map)
        return hashmap_result_bad_argument;

    item = _find_entry(map, key, key_len);
    if(!item)
        return hashmap_result_bad_key;

    *result = item->value;
    return hashmap_result_success;
}


hashmap_result hashmap_get_u32(hashmap_t *map, uint32_t key, uintptr_t *result)
{
    char as_bytes[4] = u32_byte_string(key);
    return hashmap_get(map, as_bytes, 4, result);
}


hashmap_result hashmap_destroy(hashmap_t *map)
{
    map_item_t *curr_item, *temp_item;

    if(!map)
        return hashmap_result_bad_argument;


    for(uint32_t i = 0; i < map->capacity; i++)
    {
        if(map->buckets[i] != 0)
        {
            curr_item = map->buckets[i];
            do{
                temp_item = curr_item;
                curr_item = curr_item->next;

                free(temp_item);
            }while(curr_item != NULL);
        }
    }

    free(map->buckets);
    free(map);

    return hashmap_result_success;
}

