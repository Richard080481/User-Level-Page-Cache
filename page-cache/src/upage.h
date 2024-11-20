#ifndef USER_PAGE_CACHE
#define USER_PAGE_CACHE
#define CACHE_SIZE 1000
#define PAGE_HEADER_SIZE sizeof(Header)

#include <cache_api.h>
#include <spdk.h>

typedef struct
{
    unsigned int PAGES;         // Length of the data in the package
} Header;

typedef struct PAGE
{
    unsigned flag;
    char* path_name;
    unsigned int index;
    struct PAGE* prev;
    struct PAGE* next;
}page;

enum pageflags
{
    PG_locked = 0x01,
    PG_dirty = 0x02,
    PG_lru = 0x04,
};

struct lru_cache
{
    page* head;
    page* tail;
    int nr_pages;
};

struct page_free_list
{
    page* head;
    int nr_free;
};

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
 * @brief Move a page to the head of the LRU list
 * @return No return value
 */
void move_to_lru_head(page* page);

/**
 * @brief Write to page cache
 * @return 0, if success
 *         non-zero, if fail
 */
int page_cache_write(char* path_name, char* data);

/**
 * @brief send pio(write) to dm-cache
 * @return No return value
 */
void write_pio(page* p);

/**
 * @brief send pio(read) to dm-cache
 * @return No return value
 */
void read_pio(page* p);
#endif

