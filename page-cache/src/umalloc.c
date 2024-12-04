#include "umalloc.h"
#include <spdk/env.h>
#include <spdk/memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

void* umalloc_dma(size_t len)
{
    return malloc(len);
}

void* umalloc_share(char* shm_name, size_t len)
{
    return malloc(len);
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return NULL;
    }
    if (ftruncate(shm_fd, len) == -1) {
        perror("ftruncate");
        close(shm_fd);  // Close file descriptor before returning.
        return NULL;
    }
    void* shm_ptr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);  // Close file descriptor before returning.
        return NULL;
    }
    close(shm_fd);  // Close file descriptor after mapping.
    return shm_ptr;
}

void ufree(void* free_addr)
{
    free(free_addr);
}

// void* link_shm(char* shm_name, size_t shm_size) {
//     int shm_fd = shm_open(shm_name, O_RDWR, 0666);
//     if (shm_fd == -1) {
//         perror("shm_open");
//         return NULL;
//     }
//     void* shm_ptr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
//     if (shm_ptr == MAP_FAILED) {
//         perror("mmap");
//         close(shm_fd);  // Close file descriptor before returning.
//         return NULL;
//     }
//     close(shm_fd);  // Close file descriptor after mapping.
//     return shm_ptr;
// }

// int unmap_shm(void* shm_ptr, size_t shm_size)
// {
//     if (munmap(shm_ptr, shm_size) == -1) {
//         perror("munmap");
//         return 1;
//     }
//     return 0;
// }

// int unlink_shm(char* shm_name) {
//     if (shm_unlink(shm_name) == -1)
//     {
//         perror("shm_unlink");
//         return 1;
//     }
//     return 0;
// }