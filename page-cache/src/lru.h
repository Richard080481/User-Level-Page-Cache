#ifndef LRU_H
#define LRU_H

#include "upage.h"

typedef struct lru_entry
{
    page* page_ptr;
    struct lru_entry* prev;
    struct lru_entry* next;
} lru_entry;

typedef struct lru_cache
{
    lru_entry* head;
    lru_entry* tail;
    int nr_pages;
} lru_cache;

/**
 * @brief Move a page to the head of the LRU list
 * @return No return value
 */
void add_to_lru_head(lru_cache* lru_list, page* page);

/**
 * @brief Move a page which has already in the lru list to the head of the LRU list
 * @return No return value
 */
void move_to_lru_head(lru_cache* lru_list, page* page);

/**
 * @brief Remove a page which is in the lru list
 * @return No return value
 */
void remove_from_lru(lru_cache* lru_list, page* page);
#endif