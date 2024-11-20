#include "umalloc.h"
#include <spdk/env.h>
#include <spdk/memory.h>

void* umalloc_share(unsigned len)
{
    return spdk_zmalloc(len, 0x1000, NULL, SPDK_ENV_NUMA_ID_ANY, SPDK_MALLOC_SHARE);
}

void* umalloc_dma(unsigned len)
{
    return spdk_zmalloc(len, 0x1000, NULL, SPDK_ENV_NUMA_ID_ANY, SPDK_MALLOC_DMA);
}

void ufree(void* free_addr)
{
    spdk_free(free_addr);
}
