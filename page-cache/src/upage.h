#ifndef USER_PAGE_CACHE
#define USER_PAGE_CACHE



#include <cache_api.h>
#include <spdk.h>
#include "lru.h"
#include "upio.h"

typedef struct PAGE
{
    unsigned flag;
    char* path_name;
    unsigned int index;
    struct PAGE* prev;
    struct PAGE* next;
    lru_entry* lru_ptr;
}page;

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
void free_page(page* p);

/**
 * @brief Write to page cache
 * @return 0, if success
 *         non-zero, if fail
 */
int page_cache_write(char* path_name, char* data);

/**
 * @brief Write to user's buffer
 * @return No return value
 */
void write_to_buffer(page* page, void* buffer);
#endif