#include "cache_api.h"
#include "upage.h"
#include "spdk.h"

void write_pio(page* pg, void* PHYS_BASE, page* mem_map)
{
    /* get the number of pages for the given path */
    void* page_data_addr = ((char*)PHYS_BASE) + ((pg - mem_map) * PAGE_SIZE);

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

    struct pio* head = create_pio(pg->path_name, 0, pg->index, operation, page_data_addr, 1);
    submit_pio(head);
    free_pio(head);

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

    return 0;
}