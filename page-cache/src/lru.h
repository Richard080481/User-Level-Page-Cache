#ifndef LRU_H
#define LRU_H

#include "types.h"

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