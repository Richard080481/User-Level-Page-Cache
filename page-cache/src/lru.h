#ifndef LRU_H
#define LRU_H

#include "upage.h"

hash_entry* hash_table[CACHE_SIZE] = {NULL};

typedef struct lru_entry
{
    page* page_ptr;
    struct lru_entry* prev;
    struct lru_entry* next;
} lru_entry;

typedef struct hash_entry
{
    lru_entry* lru_entry_ptr;
    struct hash_entry* next;
} hash_entry;

typedef struct lru_cache
{
    lru_entry* head;
    lru_entry* tail;
} lru_cache;

/**
 * @brief Move a page to the head of the LRU list
 * @return No return value
 */
void add_to_lru_head(lru_cache* lru_list, page* pg)

/**
 * @brief Move a page which has already in the lru list to the head of the LRU list
 * @return No return value
 */
void move_to_lru_head(lru_cache* lru_list, lru_entry* page);

/**
 * @brief Remove a page which is in the lru list
 * @return No return value
 */
void remove_from_lru(lru_cache* lru_list, lru_entry* page);
#endif