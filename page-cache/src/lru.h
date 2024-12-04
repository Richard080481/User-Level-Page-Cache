#ifndef LRU_H
#define LRU_H

#include "umalloc.h"
#include "utypes.h"

#define CACHE_SIZE 1000

extern hash_entry* hash_table[CACHE_SIZE];

/**
 * @brief Move a page to the head of the LRU list
 * @return No return value
 */
void add_to_lru_head(lru_cache* lru_list, page* pg);

/**
 * @brief Move a page which has already in the lru list to the head of the LRU list
 * @return No return value
 */
void move_to_lru_head(lru_cache* lru_list, lru_entry* hd);

/**
 * @brief Remove a page which is in the lru list
 * @return No return value
 */
int remove_from_lru(lru_cache* lru_list, lru_entry* hd);

/**
 * @brief Count the hash value of the given path name
 * @return Hash value of the path name
 */
unsigned int hash_function(char* path_name);

/**
 * @brief Check whether the path name is in the hash table
 * @return The address of the hash entry for the path name
 */
hash_entry* hash_table_lookup(char* path_name);

/**
 * @brief Insert the LRU entry into the hash table
 * @return No return value
 */
void hash_table_insert(lru_entry* hd);

/**
 * @brief Remove the LRU entry from the hash table
 * @return 0, if success
 *         non-zero, if fail
 */
int hash_table_remove(lru_entry* hd);
#endif