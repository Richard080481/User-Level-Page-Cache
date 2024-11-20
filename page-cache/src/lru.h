#ifndef LRU_H
#define LRU_H

#include "types.h"

/**
 * @brief Move a page to the head of the LRU list
 * @return No return value
 */
void add_to_lru_head(page* page);
#endif