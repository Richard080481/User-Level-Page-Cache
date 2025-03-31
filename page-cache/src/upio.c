#include "cache_api.h"
#include "upage.h"
#include "spdk.h"

void write_pio(page* pg, void* PHYS_BASE, page* mem_map)
{
    /* get the number of pages for the given path */
    // header* hd = umalloc_dma(sizeof(header));
    char* len = (char*)umalloc_dma(PAGE_HEADER_SIZE);
    unsigned int page_cnt = 0;
    void* page_data_addr = ((char*)PHYS_BASE) + ((pg - mem_map) * PAGE_SIZE);
    memcpy(len, page_data_addr, PAGE_HEADER_SIZE);
    page_cnt = convert_string_to_unsigned_int(len);
    free_dma_buffer(len);

    /* creat pio head */
    operate operation = WRITE;
    // struct pio* head = create_pio(pg->path_name, 0, pg->index, operation, page_data_addr, page_cnt); // creat pio for the first page
    // /* implement pio append */
    // page* next_page = pg->next;
    // for (unsigned int i = 1; i < page_cnt; i++)
    // {
    //     page_data_addr = ((char*)PHYS_BASE) + ((next_page - mem_map) * PAGE_SIZE);
    //     printf("page = %s\n", (char*)page_data_addr);
    //     append_pio(head, page_data_addr);
    //     next_page = next_page->next;
    // }

    // submit_pio(head);
    // free_pio(head);


    struct pio* head;
    page* next_page = pg;
    for (unsigned int i = 1; i < page_cnt; i++)
    {
        head = create_pio(pg->path_name, 0, next_page->index, operation, page_data_addr, 1);
        submit_pio(head);
        free_pio(head);
        next_page = next_page->next;
        page_data_addr = ((char*)PHYS_BASE) + ((next_page - mem_map) * PAGE_SIZE);
    }


    // operation = READ;
    // char* buf = (char*)umalloc_dma(PAGE_SIZE);
    // struct pio* head2 = create_pio(pg->path_name, 0, 1, operation, buf, 1); // creat pio for the first page
    // submit_pio(head2);
    // free_pio(head2);
    // memset(buf + PAGE_SIZE - 1, '\0', 1);
    // printf("read pio page data = %s\n", (char*)buf);
    // ufree(buf);
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
    // header* hd = umalloc_dma(sizeof(header));
    char* len = (char*)umalloc_dma(PAGE_HEADER_SIZE);
    unsigned int page_cnt = 0;
    memcpy(len, page_data_addr, PAGE_HEADER_SIZE);
    page_cnt = convert_string_to_unsigned_int(len);
    free_dma_buffer(len);

    if (page_cnt > CACHE_SIZE) {return page_cnt;} // if the file is larger than the cache size, do not put it in the page cache

    /* setting infomation of new page */
    page* prev_page = pg;

    /* implement pio append */
    for (unsigned int i = 1; i < page_cnt; i++)
    {
        /* set the data of new page */
        page* new_page = alloc_page();
        prev_page->next = new_page;
        new_page->flag |= PG_lru;
        new_page->index = i;
        new_page->path_name = path_name;
        new_page->next = NULL;

        page_data_addr = ((char*)PHYS_BASE) + ((new_page - mem_map) * PAGE_SIZE);
        head = create_pio(path_name, 0, new_page->index, operation, page_data_addr, 1);
        submit_pio(head);
        free_pio(head);
        prev_page = new_page;
    }

    return 0;
}