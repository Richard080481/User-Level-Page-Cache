#ifndef UMALLOC_H
#define UMALLOC_H

#include <spdk.h>  // Include SPDK for any specific memory allocation and management functions

/**
 * @brief  Allocate sharable memory.
 *
 * This function allocates a shared memory region that can be accessed by multiple processes.
 * The memory is allocated based on the provided `shm_name` and size `len`.
 *
 * @param shm_name The name of the shared memory object to allocate.
 * @param len The size of the memory to allocate in bytes.
 * @return A pointer to the allocated shared memory if successful, or NULL if no free space is available or allocation fails.
 */
void* umalloc_share(char* shm_name, size_t len);

/**
 * @brief  Allocate DMA (Direct Memory Access) memory.
 *
 * This function allocates memory that can be used for direct memory access operations.
 * It ensures the allocated memory is suitable for efficient data transfers between the CPU and peripherals.
 *
 * @param len The size of the memory to allocate in bytes.
 * @return A pointer to the allocated DMA memory if successful, or NULL if allocation fails.
 */
void* umalloc_dma(size_t len);

/**
 * @brief  Free allocated memory.
 *
 * This function releases memory that was previously allocated using `umalloc_share()` or `umalloc_dma()`.
 * After calling this function, the memory is no longer accessible.
 *
 * @param free_addr A pointer to the memory to be freed.
 */
void ufree(void* free_addr);

/**
 * @brief  Link to an existing shared memory region.
 *
 * This function links to an already created shared memory region by name and maps it to the process's address space.
 * It does not create a new shared memory region but allows access to an existing one.
 *
 * @param shm_name The name of the shared memory object to link to.
 * @param shm_size The size of the memory region to link to in bytes.
 * @return A pointer to the mapped shared memory if successful, or NULL if the mapping fails.
 */
void* link_shm(char* shm_name, size_t shm_size);

/**
 * @brief  Unmap a shared memory region.
 *
 * This function unmaps the shared memory region that was previously mapped using `link_shm()`.
 * After this operation, the region is no longer accessible in the process's address space.
 *
 * @param shm_ptr A pointer to the memory region to unmap.
 * @param shm_size The size of the memory region to unmap in bytes.
 * @return 0 if successful, or a non-zero value if an error occurs.
 */
int unmap_shm(void* shm_ptr, size_t shm_size);

/**
 * @brief  Unlink a shared memory region.
 *
 * This function removes a shared memory object from the system, so it can no longer be accessed by name.
 * However, the memory region is only deleted once all processes unmap it.
 *
 * @param shm_name The name of the shared memory object to unlink.
 * @return 0 if successful, or a non-zero value if an error occurs.
 */
int unlink_shm(char* shm_name);

#endif  // UMALLOC_H
