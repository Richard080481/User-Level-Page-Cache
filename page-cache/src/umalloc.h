#include <spdk.h>

/**
 * @brief  Allocate sharable memory
 * @return address, if success
 *         NULL, if no free space
 */
void* umalloc_share(unsigned len);

/**
 * @brief  Allocate dma memory
 * @return address, if success
 *         NULL, if no free space
 */
void* umalloc_dma(unsigned len);

/**
 * @brief  Free memory
 * @return No return value
 */
void ufree(void* free_addr);