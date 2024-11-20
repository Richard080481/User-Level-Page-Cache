#include "upio.h"
#include "upage.h"
#include <spdk/env.h>
#include <spdk/memory.h>

void write_pio(page* p, void* PHYS_BASE, page* mem_map)
{
    unsigned int page_cnt = 0;
    void* page_data_addr = ((char*)PHYS_BASE) + ((p - mem_map) * PAGE_SIZE);
    memcpy(page_cnt, page_data_addr, PAGE_HEADER_SIZE);
    // implement pio append
    operate operation = WRITE;
    struct pio* head = create_pio(p->path_name, 0, page->index, operation, page_data_addr, 1); // submit pio to write the page into ssd, but page index is not set yet
    submit_pio(head);
    free_pio(head);
}

void read_pio(page* p)
{
    void* page_data_addr = ((char*)PHYS_BASE) + ((p - mem_map) * PAGE_SIZE);
    operate operation = READ;
    struct pio* head = create_pio(p->path_name, 0, p->index, operation, page_data_addr, 1); // submit pio to write the page into ssd, but page index is not set yet
    submit_pio(head);
    free_pio(head);
}