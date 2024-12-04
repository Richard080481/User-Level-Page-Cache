#ifndef USER_PAGE_CACHE
#define USER_PAGE_CACHE

#include <cache_api.h>
#include <spdk.h>
#include "utypes.h"
#include "lru.h"
#include "upio.h"
#include "umalloc.h"

#define PAGE_HEADER_SIZE sizeof(header)

/**
 * @brief Init share memory, wakeup workers
 * @return 0, if success
 *         non-zero, if fail
 */
int init_page_cache(void);

/**
 * @brief Unlink share memory, shutdown workers
 * @return 0, if success
 *         non-zero, if fail
 */
int exit_page_cache(void);

/**
 * @brief Map to share memory
 * @return 0, if success
 *         non-zero, if fail
 */
int link_page_cache(void);

/**
 * @brief Unmap share memory
 * @return 0, if success
 *         non-zero, if fail
 */
int unlink_page_cache(void);

/**
 * @brief Force to unlink share memory, lockless (for debug)
 * @return No return value
 */
void force_exit_page_cache(void);

/**
 * @brief Print page-cache mapping information
 * @return No return value
 */
void info_page_cache(void);

/**
 * @brief Allocate a new page
 * @return Page's address, if success
 *         NULL, if fail
 */
page* alloc_page(void);

/**
 * @brief Free the page, put it back to free page list
 * @return No return value
 */
void free_page(page* page);

/**
 * @brief Write to page cache
 * @return 0, if success
 *         non-zero, if fail
 */
int uwrite(char* path_name, char* data);

/**
 * @brief Read from page cache
 * @return 0, if success
 *         non-zero, if fail
 */
int uread(char* path_name, void* buffer);

/**
 * @brief Write to user's buffer
 * @return No return value
 */
void write_to_buffer(page* page, void* buffer);
#endif