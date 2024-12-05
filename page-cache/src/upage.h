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
 * @brief Initialize shared memory and wake up worker threads.
 *
 * This function sets up the shared memory and starts the worker threads that
 * handle operations related to the page cache.
 *
 * @return 0 if the initialization is successful, non-zero if it fails.
 */
int init_page_cache(void);

/**
 * @brief Unlink shared memory and shut down worker threads.
 *
 * This function cleans up by disconnecting from shared memory and stopping
 * any running worker threads associated with the page cache.
 *
 * @return 0 if the shutdown is successful, non-zero if it fails.
 */
int exit_page_cache(void);

/**
 * @brief Link to shared memory.
 *
 * This function connects the process to the shared memory that holds the
 * page cache data.
 *
 * @return 0 if the linking is successful, non-zero if it fails.
 */
int link_page_cache(void);

/**
 * @brief Unlink shared memory.
 *
 * This function disconnects the process from the shared memory, effectively
 * ending the session with the page cache.
 *
 * @return 0 if the unlinking is successful, non-zero if it fails.
 */
int unlink_page_cache(void);

/**
 * @brief Forcefully unlink shared memory (used for debugging).
 *
 * This function unlinks the shared memory without locking, allowing for
 * immediate disconnection from the page cache. It is intended for debugging
 * purposes.
 *
 * @return No return value.
 */
void force_exit_page_cache(void);

/**
 * @brief Print information about the page cache mapping.
 *
 * This function outputs the current state and layout of the page cache
 * mapping for debugging or monitoring purposes.
 *
 * @return No return value.
 */
void info_page_cache(void);

/**
 * @brief Allocate a new page in the page cache.
 *
 * This function allocates a page from the page cache. If successful, the
 * address of the allocated page is returned.
 *
 * @return Pointer to the allocated page if successful, NULL if allocation fails.
 */
page* alloc_page(void);

/**
 * @brief Free a page and return it to the free page list.
 *
 * This function frees the specified page and returns it to the list of free
 * pages available for reuse in the page cache.
 *
 * @param page Pointer to the page to be freed.
 *
 * @return No return value.
 */
void free_page(page* page);

/**
 * @brief Open a file for reading or writing.
 *
 * This function opens a file and returns a user-defined file pointer (`uFILE`)
 * for subsequent read or write operations on the file.
 *
 * @param filename The name of the file to be opened.
 * @param mode The mode in which the file is to be opened (e.g., "r", "w", "rw").
 *
 * @return A pointer to the opened file stream if successful, NULL if the
 *         operation fails.
 */
uFILE* uopen(const char* filename, const char* mode);

/**
 * @brief Close the opened file stream.
 *
 * This function closes the specified user-defined file stream, releasing
 * any resources associated with the stream.
 *
 * @param stream The user-defined file stream to be closed.
 *
 * @return 0 if successful, non-zero if the operation fails.
 */
int uclose(uFILE* stream);

/**
 * @brief Write data to the page cache.
 *
 * This function writes data to the page cache associated with a specified
 * file (identified by its path name).
 *
 * @param path_name The path name of the file to be written to.
 * @param data The data to be written to the page cache.
 *
 * @return 0 if the write is successful, non-zero if the operation fails.
 */
int uwrite(char* path_name, char* data);

/**
 * @brief Read data from the page cache.
 *
 * This function reads data from the page cache into a specified buffer for
 * the file identified by its path name.
 *
 * @param path_name The path name of the file to read from.
 * @param buffer The buffer where the read data will be stored.
 *
 * @return 0 if the read is successful, non-zero if the operation fails.
 */
int uread(char* path_name, void* buffer);

/**
 * @brief Write the contents of a page to the user's buffer.
 *
 * This function copies the contents of a specified page into a buffer provided
 * by the user, allowing access to the data stored in the page cache.
 *
 * @param page Pointer to the page whose contents will be written to the buffer.
 * @param buffer The buffer to which the page data will be written.
 *
 * @return No return value.
 */
void write_to_buffer(page* page, void* buffer);

#endif