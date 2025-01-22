#include "cache_api.h"
#include "upage.h"
#include "spdk.h"

void write_pio(page* pg, void* PHYS_BASE, page* mem_map)
{
    /* get the number of pages for the given path */
    header* hd = umalloc_dma(sizeof(header));
    unsigned int page_cnt = 0;
    void* page_data_addr = ((char*)PHYS_BASE) + ((pg - mem_map) * PAGE_SIZE);
    memcpy(hd, page_data_addr, PAGE_HEADER_SIZE);
    page_cnt = hd->PAGES;
    free_dma_buffer(hd);

    /* creat pio head */
    operate operation = WRITE;
    struct pio* head = create_pio(pg->path_name, 0, pg->index, operation, page_data_addr, page_cnt); // creat pio for the first page

    /* implement pio append */
    page* next_page = pg->next;
    for (unsigned int i = 1; i < page_cnt; i++)
    {
        page_data_addr = ((char*)PHYS_BASE) + ((next_page - mem_map) * PAGE_SIZE);
        append_pio(head, page_data_addr);
        next_page = next_page->next;
    }

    submit_pio(head);
    free_pio(head);
}

unsigned int read_pio(page* pg, void* PHYS_BASE, page* mem_map)
{
    /* read the first page from the dm-cache to determine the total number of pages for the given path */
    void* page_data_addr = ((char*)PHYS_BASE) + ((pg - mem_map) * PAGE_SIZE);
    operate operation = READ;
    char* path_name = pg->path_name;
    struct pio* head = create_pio(path_name, 0, pg->index, operation, page_data_addr, 1);
    submit_pio(head);
    free_pio(head);
    /* get the number of pages for the given path */
    header* hd = umalloc_dma(sizeof(header));
    unsigned int page_cnt = 0;
    memcpy(hd, page_data_addr, PAGE_HEADER_SIZE);
    page_cnt = hd->PAGES;
    free_dma_buffer(hd);

    if (page_cnt > CACHE_SIZE) {return page_cnt;} // if the file is larger than the cache size, do not put it in the page cache

    /* creat pio head (head is the second page of the path) */
    page_cnt--; // the first page has already been read

    if (page_cnt == 0) {return 0;}

    /* setting infomation of new page */
    page* next_page = alloc_page();

    pg->next = next_page; // set first page's next
    next_page->flag |= PG_lru;
    next_page->index = 1;
    next_page->path_name = path_name;
    next_page->next = NULL;

    page_data_addr = ((char*)PHYS_BASE) + ((next_page - mem_map) * PAGE_SIZE);
    head = create_pio(path_name, 0, next_page->index, operation, page_data_addr, page_cnt);

    /* implement pio append */
    for (unsigned int i = 1; i < page_cnt; i++)
    {
        /* set the data of new page */
        page* new_page = alloc_page();
        next_page->next = new_page;
        new_page->flag |= PG_lru;
        new_page->index = i+1;
        new_page->path_name = path_name;
        new_page->next = NULL;

        page_data_addr = ((char*)PHYS_BASE) + ((new_page - mem_map) * PAGE_SIZE);
        append_pio(head, page_data_addr);
        next_page = new_page;
    }
    submit_pio(head);
    free_pio(head);

    return 0;
}