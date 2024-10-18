#ifndef USER_PAGE_CACHE
#define USER_PAGE_CACHE

#include <cache_api.h>
#include <spdk.h>

/**
 * @brief Init share memory, wakeup workers
 * @return 0, if success
 *         non-zero, if fail
 */
int init_page_cache(void);

/**
 * @brief Unlink share memory, shotdown workers
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
#endif