#include "upio.h"
#include "upage.h"
#include <spdk/env.h>
#include <spdk/memory.h>

void write_pio(page* p, void* PHYS_BASE, page* mem_map)
{
    /* get the number of pages for the given path */
    unsigned int page_cnt = 0;
    void* page_data_addr = ((char*)PHYS_BASE) + ((p - mem_map) * PAGE_SIZE);
    memcpy(&page_cnt, page_data_addr, PAGE_HEADER_SIZE);

    /* creat pio head */
    operate operation = WRITE;
    struct pio* head = create_pio(p->path_name, 0, p->index, operation, page_data_addr, page_cnt); // creat pio for the first page

    /* implement pio append */
    page* next_page = p->next;
    for (int i = 1; i < page_cnt; i++)
    {
        page_data_addr = ((char*)PHYS_BASE) + ((next_page - mem_map) * PAGE_SIZE);
        append_pio(head, page_data_addr);
        next_page = next_page->next;
    }

    submit_pio(head);
    free_pio(head);
}

void read_pio(page* p, void* PHYS_BASE, page* mem_map)
{
    /* read the first page from the dm-cache to determine the total number of pages for the given path */
    void* page_data_addr = ((char*)PHYS_BASE) + ((p - mem_map) * PAGE_SIZE);
    operate operation = READ;
    struct pio* head = create_pio(p->path_name, 0, p->index, operation, page_data_addr, 1);
    submit_pio(head);
    free_pio(head);

    /* get the number of pages for the given path */
    unsigned int page_cnt = 0;
    memcpy(&page_cnt, page_data_addr, PAGE_HEADER_SIZE);

    /* creat pio head (head is the second page of the path) */
    operate operation = READ;
    page_cnt--; // the first page has already been read

    page* next_page = p->next;
    page_data_addr = ((char*)PHYS_BASE) + ((next_page - mem_map) * PAGE_SIZE);
    struct pio* head = create_pio(next_page->path_name, 0, next_page->index, operation, page_data_addr, page_cnt);

    /* implement pio append */
    next_page = next_page->next;
    for (int i = 1; i < page_cnt; i++)
    {
        page_data_addr = ((char*)PHYS_BASE) + ((next_page - mem_map) * PAGE_SIZE);
        append_pio(head, page_data_addr);
        next_page = next_page->next;
    }
    submit_pio(head);
    free_pio(head);
}