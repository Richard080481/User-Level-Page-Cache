#ifndef LRU_H
#define LRU_H

#include "umalloc.h"
#include "utypes.h"

#define CACHE_SIZE  262145 // (131072 = 0.5 GB) Define the size of the cache (hash table size)

extern hash_entry* hash_table[CACHE_SIZE];  // Global hash table to store the cache entries

/**
 * @brief add a new page to the head of the LRU (Least Recently Used) list.
 *
 * This function moves the specified page to the head of the LRU list
 * making it the most recently used.
 *
 * @param lru_list The LRU cache list.
 * @param pg The page to be moved to the head of the LRU list.
 *
 * @return No return value.
 */
void add_to_lru_head(lru_cache* lru_list, page* pg);

/**
 * @brief Move a page already in the LRU list to the head of the LRU list.
 *
 * If a page is already in the LRU list and has been accessed, it needs
 * to be moved to the head to mark it as recently used.
 *
 * @param lru_list The LRU cache list.
 * @param hd The entry in the LRU list that should be moved to the head.
 *
 * @return No return value.
 */
void move_to_lru_head(lru_cache* lru_list, page* pg);

/**
 * @brief Remove a page from the LRU list.
 *
 * This function removes a specified page from the LRU list, making it
 * no longer part of the cache.
 *
 * @param lru_list The LRU cache list.
 * @param hd The entry to be removed from the LRU list.
 *
 * @return 0 if the removal is successful, non-zero if it fails.
 */
int remove_from_lru(lru_cache* lru_list, page *pg);

// void print_all_pages(page* head);

// void print_lru_entry(lru_entry* entry);

// void print_lru_cache(lru_cache* cache);

/**
 * @brief Count the hash value for the given path name.
 *
 * This function computes the hash value for a given path name, which
 * is used to locate the corresponding entry in the hash table.
 *
 * @param path_name The path name to compute the hash value for.
 *
 * @return The computed hash value for the path name.
 */
unsigned int hash_function(char* path_name, unsigned int index);

/**
 * @brief Check whether the path name is present in the hash table.
 *
 * This function searches the hash table to check if the given path name
 * is already present, returning the corresponding hash entry if found.
 *
 * @param path_name The path name to look for in the hash table.
 *
 * @return The address of the hash entry for the path name, or NULL if not found.
 */
hash_entry* hash_table_lookup(char* path_name, unsigned int index);

/**
 * @brief Insert the LRU entry into the hash table.
 *
 * This function inserts a new entry into the hash table, associating
 * it with a path name and a corresponding cache entry.
 *
 * @param hd The LRU entry to insert into the hash table.
 *
 * @return No return value.
 */
void hash_table_insert(page* pg);

/**
 * @brief Remove an LRU entry from the hash table.
 *
 * This function removes the specified entry from the hash table,
 * effectively removing it from the cache.
 *
 * @param hd The LRU entry to remove from the hash table.
 *
 * @return 0 if the removal is successful, non-zero if it fails.
 */
int hash_table_remove(page* pg);

#endif
