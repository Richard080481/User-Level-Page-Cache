#include "upage.h"
#include <spdk/env.h>
#include <spdk/memory.h>
#include <string.h>
#include <error.h>

struct lru_cache lru_cache;
unsigned int current_offset = 0;

int init_page_cache(void)
{
    if(init_ssd_cache())
    {
        perror("Error init ssd cache\n");
	return 1;
    }

    return 0;

}

int exit_page_cache(void)
{
    if(lru_cache.nr_pages != 0)
    {
        page *freePage = lru_cache.tail;
        if (lru_cache.tail->prev != NULL) lru_cache.tail->prev->next = NULL;
        lru_cache.tail = lru_cache.tail->prev;
        spdk_free(freePage);
        lru_cache.nr_pages--;        
    }

    exit_ssd_cache();
    return 0;
}

page* alloc_page(void) 
{
    /*(not finished yet!)*/
    /*if (lru_cache.nr_pages > CACHE_SIZE) //the number of pages reaches the maximum limit, do write-back
    {    
        page *evict = lru_cache.tail;
        if (evict) {

            if (lru_cache.tail->prev) lru_cache.tail->prev->next = NULL;
            lru_cache.tail = lru_cache.tail->prev;

            struct pio* head = create_pio(FILE_NAME, 0, page_index, READ, evict, 1);//submit pio to write evict page into ssd, but page index is not set yet
            submit_pio(head);
            free_pio(head);

            spdk_free(evict);
            cache.nr_pages--;
        }
    }*/

    page *newPage = (page*)spdk_zmalloc(PAGE_SIZE, 0x1000, NULL, SPDK_ENV_NUMA_ID_ANY, SPDK_MALLOC_SHARE);
    if (!newPage) {
        perror("Failed to allocate new page");
        return NULL;
    }

    newPage->flag = 0;
    lru_cache.nr_pages++;

    return newPage;
}

void move_to_lru_head(page *page)
{
    page->next = lru_cache.head;
    if (lru_cache.head) lru_cache.head->prev = page;
    lru_cache.head = page;
    if (!lru_cache.tail) lru_cache.tail = page;
}

void page_cache_write(char *data) 
{
    unsigned int len = strlen(data);
    unsigned int data_offset = 0;

    while (data_offset < len)
    {
        page *newPage = alloc_page();
        if(!newPage) return;

        unsigned int copyLen = (len - data_offset) < PAGE_SIZE ? (len - data_offset) : PAGE_SIZE;// the number of bytes to be written this time
        memcpy(newPage, data + data_offset, copyLen);
        newPage->flag |= PG_dirty;
        move_to_lru_head(newPage);
        data_offset += copyLen;
    }
    return;
}

int main(void)
{
    init_page_cache();
    void *temp = malloc(PAGE_SIZE);
    sprintf(temp, "%d", rand() % 1000000);
    page_cache_write(temp);
    exit_page_cache();
    return 0;
}
