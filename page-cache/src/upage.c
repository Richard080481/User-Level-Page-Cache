#include "upage.h"
#include "umalloc.h"
#include <spdk/env.h>
#include <spdk/memory.h>
#include <string.h>
#include <error.h>

#define PAGE_HEADER_SIZE sizeof(header)
#define CACHE_SIZE 1000

#define ENABLE_BRANCH_HINTS
#ifdef ENABLE_BRANCH_HINTS
    // Define likely and unlikely macros
    #define likely(x)   __builtin_expect(!!(x), 1)
    #define unlikely(x) __builtin_expect(!!(x), 0)
#else
    // When branch hints are disabled, likely/unlikely do nothing
    #define likely(x)   (x)
    #define unlikely(x) (x)
#endif


page* mem_map;
void* PHYS_BASE;
lru_cache lru_list = {NULL, NULL, 0}; // warning!!!
struct page_free_list free_list = {NULL, 0};

int init_page_cache(void)
{
    force_exit_ssd_cache();
    if (init_ssd_cache())
    {
        perror("Error init ssd cache\n");
	    return 1;
    }

    mem_map = (page*)umalloc_share(CACHE_SIZE * sizeof(page)); // allocate space for struct PAGE
    PHYS_BASE =  umalloc_share(CACHE_SIZE * PAGE_SIZE);

    /* put all free pages int free list */
    for (int i = 0;i < CACHE_SIZE;i++)
    {
        mem_map[i].next = free_list.head;
        if (free_list.head)
        {
            free_list.head->prev = &mem_map[i];
        }
        free_list.head = &mem_map[i];

        free_list.nr_free++;
    }
    return 0;
}

int exit_page_cache(void)
{
    ufree(mem_map);
    ufree(PHYS_BASE);
    exit_ssd_cache();
    return 0;
}

void write_pio(page* p)
{
    void* page_data_addr = ((char*)PHYS_BASE) + ((p - mem_map) * PAGE_SIZE);
    operate operation = WRITE;
    struct pio* head = create_pio(p->path_name, 0, p->index, operation, page_data_addr, 1); // submit pio to write the page into ssd, but page index is not set yet
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

page* alloc_page(void)
{

    if (free_list.nr_free == 0) //the number of free page is zero, do write-back
    {
        page* evict = lru_list.tail;
        evict->prev->next = NULL;
        lru_list.tail = evict->prev;

        write_pio(evict); // write the page into ssd
        free_page(evict); // free the page

        lru_list.nr_pages--;

    }

    /* allocate a new page from the head of free list */
    page* new_page = free_list.head;
    free_list.head = new_page->next;
    if (free_list.head)
    {
        free_list.head->prev = NULL;
    }
    free_list.nr_free--;

    if(unlikely(!new_page))
    {
        printf("Error: allocate a page failed\n");
    }

    return new_page;
}

void free_page(page* p)
{
    /* move page to the head of free list */
    p->next = free_list.head;
    if (free_list.head)
    {
        free_list.head->prev = p;
    }
    free_list.head = p;

    /* clear all data in the page */
    p->prev = NULL;
    p->next = NULL;
    p->path_name = NULL;
    p->index = 0;
    p->flag = 0;

    free_list.nr_free++;
}

int page_cache_write(char* path_name, char* data)
{
    const unsigned int DATA_LEN = strlen(data);
    const unsigned int DATA_PAGES = (strlen(data) + PAGE_HEADER_SIZE + PAGE_SIZE - 1) / PAGE_SIZE ; // ceiling division to calculate the pages
    unsigned int data_offset = 0;
    unsigned int index = 0;
    bool isLastPage = false;
    bool isFirstPage = true;
    while (isLastPage == false) // The data has not yet been written
    {
        unsigned int copy_len;
        page* new_page = alloc_page();
        void* page_data_addr = ((char*)PHYS_BASE) + ((new_page - mem_map) * PAGE_SIZE);

        if (unlikely(!new_page)) {return -1;} // failed to get a new page, return

        /* setting infomation of new page */
        new_page->flag |= PG_dirty;
        new_page->index = index;
        new_page->path_name = path_name;

        /* Write the data into new page */
        if (unlikely(isFirstPage == true))
        {
            isFirstPage = false;
            header hd;
            hd.PAGES = DATA_PAGES;

            // If the first page is also last page
            if (unlikely(DATA_LEN + PAGE_HEADER_SIZE <= PAGE_SIZE)) {isLastPage = true;}

            copy_len = (isLastPage) ? DATA_LEN : PAGE_SIZE - PAGE_HEADER_SIZE;

            // Copy the header into the package
            memcpy(page_data_addr, &hd, PAGE_HEADER_SIZE);

            // Copy the data chunk into the package
            memcpy(page_data_addr + PAGE_HEADER_SIZE, data, copy_len);
            data_offset += (copy_len + PAGE_HEADER_SIZE);
            continue;
        }

        // the number of bytes to be written this time
        if((unlikely(DATA_LEN - data_offset <= PAGE_SIZE)))
        {
            copy_len = (DATA_LEN - data_offset);
            isLastPage = true;
        }
        else
        {
            copy_len = PAGE_SIZE;
            isLastPage = false;
        }

        memcpy(page_data_addr, data + data_offset, copy_len);

        add_to_lru_head(new_page);
        data_offset += copy_len;
        index++;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    init_page_cache();
    void* temp_write = malloc(PAGE_SIZE);
    sprintf(temp_write, "%d", rand() % 1000000);
    page_cache_write("test_file", temp_write);
    exit_page_cache();
    return 0;
}