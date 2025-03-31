#ifndef USER_PAGE_CACHE
#define USER_PAGE_CACHE

#include <cache_api.h>
#include <spdk.h>
#include "utypes.h"
#include "lru.h"
#include "upio.h"
#include "umalloc.h"

#define PAGE_HEADER_SIZE 10

/**
 * @brief Initialize the shared memory for the page cache.
 *
 * This function sets up the shared memory region where the page cache will reside.
 * It allocates necessary resources and prepares the system for subsequent caching operations.
 *
 * @return 0 if the initialization is successful, non-zero if there is a failure.
 */
int init_page_cache(void);

/**
 * @brief Clean up and disconnect from the shared memory.
 *
 * This function shuts down the page cache system by disconnecting from the shared memory,
 * freeing any allocated resources.
 *
 * @return 0 if the shutdown is successful, non-zero if there is a failure.
 */
int exit_page_cache(void);

/**
 * @brief Link to the shared memory region for the page cache.
 *
 * This function connects the current process to the shared memory region that holds the
 * page cache data, allowing access to cached pages.
 *
 * @return 0 if the linking is successful, non-zero if there is a failure.
 */
int link_page_cache(void);

/**
 * @brief Unlink from the shared memory region.
 *
 * This function disconnects the current process from the shared memory region,
 * effectively ending the session with the page cache.
 *
 * @return 0 if the unlinking is successful, non-zero if there is a failure.
 */
int unlink_page_cache(void);

/**
 * @brief Forcefully unlink from shared memory (debugging only).
 *
 * This function forcibly unlinks the shared memory region without locking,
 * providing an immediate disconnection from the page cache. This is used primarily for debugging purposes.
 *
 * @return No return value.
 */
void force_exit_page_cache(void);

/**
 * @brief Print the current state and layout of the page cache.
 *
 * This function outputs information about the page cache's structure, mapping, and status
 * for debugging or monitoring purposes.
 *
 * @return No return value.
 */
void info_page_cache(void);

/**
 * @brief Allocate a new page in the page cache.
 *
 * This function allocates a new page in the page cache. If successful, the pointer to the allocated page
 * is returned, which can be used for subsequent operations.
 *
 * @return Pointer to the allocated page if successful, NULL if the allocation fails.
 */
page* alloc_page(void);

/**
 * @brief Free a page and return it to the available pool.
 *
 * This function frees a specified page and returns it to the list of free pages,
 * making it available for reuse in the page cache.
 *
 * @param page Pointer to the page to be freed.
 *
 * @return No return value.
 */
void free_page(page* page);

/**
 * @brief Open a file for reading or writing.
 *
 * This function opens a file and returns a user-defined file pointer (`uFILE`), which can be used for
 * subsequent read or write operations on the file.
 *
 * @param filename The name of the file to be opened.
 * @param mode The mode in which the file is to be opened (e.g., "r", "w", "rw").
 *
 * @return A pointer to the opened file stream if successful, NULL if the operation fails.
 */
uFILE* uopen(char* filename, const char* mode);

/**
 * @brief Close the specified user-defined file stream.
 *
 * This function closes the user-defined file stream, releasing any resources associated with it.
 *
 * @param stream The user-defined file stream to be closed.
 *
 * @return 0 if the close operation is successful, non-zero if it fails.
 */
int uclose(uFILE* stream);

/**
 * @brief Converts an unsigned int number into a string representation.
 *
 * Converts an unsigned int number into a PAGE_HEADER_SIZE string representation, with the least significant digit first. Any unused digits are padded with '0'.
 *
 * @param num The unsigned integer to be converted.
 * @param buffer A character buffer to store the resulting string.
 *
 * @return No return value.
 */
void convert_unsigned_int_to_string(unsigned int num, char* len);

/**
 * @brief Converts a reversed string back into an unsigned int.
 *
 * Converts a PAGE_HEADER_SIZE reversed string (where the least significant digit comes first) back into an unsigned int.
 *
 * @param buffer A string containing the 32-character reversed representation of a number. The string must only contain numeric characters ('0'-'9').
 *
 * @return Returns an unsigned int that represents the integer value of the string.
 */
unsigned int convert_string_to_unsigned_int(const char* buffer);

/**
 * @brief Write data to a file stream.
 *
 * This function writes data from the buffer pointed to by `ptr` to the file stream specified by `stream`.
 * The data is written in chunks of size `size`, and the total number of elements written is `nmemb`.
 * The function returns the total number of elements successfully written, which may be less than `nmemb` if an error occurs.
 *
 * @param buffer A pointer to the data to be written.
 * @param size The size (in bytes) of each element to be written.
 * @param count The number of elements to write.
 * @param stream A pointer to the `FILE` object that identifies the stream.
 *
 * @return The total number of elements successfully written. If this number differs from `nmemb`, an error may have occurred.
 */
size_t uwrite(const void* buffer, size_t size, size_t count, uFILE* stream);


/**
 * @brief Read data from the page cache.
 *
 * This function reads data from the page cache and stores it in a user-provided buffer.
 * It accesses the page cache for the file identified by its path name.
 *
 * @param path_name The path name of the file to read from.
 * @param buffer The buffer where the read data will be stored.
 *
 * @return The total number of elements successfully read. If this number differs from `nmemb`, an error may have occurred.
 */
size_t uread(void* buffer, size_t size, size_t count, uFILE* stream);

/**
 * @brief Copy the contents of a page to a user buffer.
 *
 * This function writes the data from a specified page in the page cache into a buffer provided by the user.
 * This allows the user to access the data stored in the page cache.
 *
 * @param page Pointer to the page whose contents will be written to the buffer.
 * @param buffer The buffer where the page data will be written.
 *
 * @return The total number of elements successfully written to buffer. If this number differs from `nmemb`, an error may have occurred.
 */
size_t write_to_buffer(void* buffer, size_t size, size_t count, page* page);

#endif