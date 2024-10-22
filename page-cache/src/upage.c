#include "upage.h"
#include <spdk/env.h>
#include <spdk/memory.h>
#include <string.h>
#include <error.h>

struct page *page_head = NULL;

int init_page_cache(void)
{
    if(init_ssd_cache())
    {
        perror("Error init ssd cache\n");
	return 1;
    }

    page_head = (struct page*)spdk_zmalloc(PAGE_SIZE * 100, 0x1000, NULL, SPDK_ENV_NUMA_ID_ANY, SPDK_MALLOC_SHARE);

    if(!page_head)
    {
	perror("Error: alloc_dma_buffer failed\n");
	return 1;
    }

    return 0;

}

int exit_page_cache(void)
{
    spdk_free(page_head);

    exit_ssd_cache();

    return 0;
}


int main(void)
{
    init_page_cache();
    exit_page_cache();
    return 0;
}
