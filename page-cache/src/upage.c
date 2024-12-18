#include "upage.h"
#include "umalloc.h"
#include "cache_api.h"
#include <spdk/env.h>
#include <spdk/memory.h>
#include <string.h>
#include <error.h>

typedef struct page_free_list
{
    page* head;
    int nr_free;
} page_free_list;

page* mem_map;
void* PHYS_BASE;
lru_cache lru_list = {NULL, NULL};
page_free_list free_list = {NULL, 0};

int init_page_cache(void)
{
    force_exit_ssd_cache();
    if (init_ssd_cache())
    {
        perror("Error init ssd cache\n");
	    return 1;
    }

    mem_map = (page*)umalloc_share(SHM_NAME, CACHE_SIZE * (size_t)sizeof(page)); // allocate space for struct PAGE
    PHYS_BASE =  umalloc_share(SHM_NAME, CACHE_SIZE * PAGE_SIZE);

    /* put all free pages int free list */
    for (int i = 0;i < CACHE_SIZE;i++)
    {
        mem_map[i].next = free_list.head;
        free_list.head = &mem_map[i];
        free_list.nr_free++;
    }
    printf("init_page_cache done\n");
    return 0;
}

int exit_page_cache(void)
{
    ufree(mem_map);
    ufree(PHYS_BASE);
    exit_ssd_cache();
    return 0;
}

page* alloc_page(void)
{

    if (free_list.nr_free == 0) // the number of free page is zero, do write-back
    {
        /* write back to the dm-cache (the tail of the LRU list). */
        write_pio(lru_list.tail->page_ptr, PHYS_BASE, mem_map); // write the page into ssd
        remove_from_lru(&lru_list, lru_list.tail); // remove lru entry (remove all pages of the file)

    }

    /* allocate a new page from the head of free list */
    page* new_page = free_list.head;
    free_list.head = free_list.head->next;
    free_list.nr_free--;

    if(unlikely(!new_page))
    {
        printf("Error: allocate a page failed\n");
    }
    return new_page;
}

void free_page(page* target_page)
{
    /* get the number of pages for the given path */
    page* tmp_page = target_page; // the address of the page that will be freed
    void* page_data_addr = ((char*)PHYS_BASE) + ((target_page - mem_map) * PAGE_SIZE);
    header* hd = umalloc_dma(sizeof(header));
    unsigned int page_cnt = 0;
    memcpy(hd, page_data_addr, PAGE_HEADER_SIZE);
    page_cnt = hd->PAGES;
    /* free all pages of the file */
    for (unsigned int i = 0;i < page_cnt;i++)
    {
        /* move page to the head of free list */
        tmp_page = target_page;
        tmp_page->next = free_list.head;
        free_list.head = tmp_page;

        /* clear all data in the page */
        tmp_page->path_name = NULL;
        tmp_page->index = 0;
        tmp_page->flag = 0;

        /* move to next page */
        target_page = target_page->next;
        free_list.nr_free++;
    }
    ufree(hd);
    return;
}

uFILE* uopen(char* filename, const char* mode)
{
    uFILE* puf = umalloc_dma(sizeof(uFILE));
    puf->path_name = filename;
    puf->mode = 0x00;
    if (strchr(mode, 'r')) {puf->mode |= U_OREAD;}
    if (strchr(mode, 'w')) {puf->mode |= U_OWRITE;}
    if (strchr(mode, '+')) {puf->mode |= U_REMOVE;}
    return puf;
}

int uclose(uFILE* stream)
{
    stream->path_name = "\0";
    stream->mode = U_INVALID;
    ufree(stream);
    return 0;
}

size_t uwrite(const void* buffer, size_t size, size_t count, uFILE* stream)
{
    if (((stream->mode) & U_OWRITE) == 0) // if U_OREAD is 0, this file cannot be written to; return 0
    {
        return 0;
    }

    char* path_name = stream->path_name;
    const unsigned int DATA_LEN = size * count;
    const unsigned int DATA_PAGES = (DATA_LEN + PAGE_HEADER_SIZE + PAGE_SIZE - 1) / PAGE_SIZE ; // ceiling division to calculate the pages
    unsigned int data_offset = 0;
    unsigned int index = 0;
    bool isLastPage = false;
    bool isFirstPage = true;
    page* prev_page = NULL;

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
        new_page->next = NULL;
        index++;

        /* setting page's next point to next page */
        if(isFirstPage == false) {prev_page->next = new_page;}
        prev_page = new_page;

        /* Write the data into new page */
        if (unlikely(isFirstPage == true))
        {
            isFirstPage = false;
            header* hd = (header*)umalloc_dma(sizeof(header));
            hd->PAGES = DATA_PAGES;
            // If the first page is also last page
            if (unlikely(DATA_LEN + PAGE_HEADER_SIZE <= PAGE_SIZE)) {isLastPage = true;}

            copy_len = (isLastPage) ? DATA_LEN : PAGE_SIZE - PAGE_HEADER_SIZE;

            // Copy the header into the package
            memcpy(page_data_addr, hd, PAGE_HEADER_SIZE);

            // Copy the data chunk into the package
            memcpy(page_data_addr + PAGE_HEADER_SIZE, buffer, copy_len);

            data_offset = copy_len;
            ufree(hd);
            add_to_lru_head(&lru_list, new_page);
            // printf("--%d---\n%s\n", index-1,(char*)(page_data_addr + PAGE_HEADER_SIZE));
            if(unlikely(DATA_LEN + PAGE_HEADER_SIZE <= PAGE_SIZE))
            {
                break;
            }
            else
            {
                continue;
            }
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

        memcpy(page_data_addr, buffer + data_offset, copy_len);

        // printf("--%d---\ndata offset = %d\ncopy len = %d\n%s\n", index-1,data_offset, copy_len, (char*)page_data_addr);

        data_offset += copy_len;
    }
    print_lru_cache(&lru_list);
    return count;
}

// for uread
size_t write_to_buffer(void* buffer, size_t size, size_t count, page* page)
{
    header* hd = (header*)umalloc_dma(sizeof(header));
    unsigned int page_cnt = 0; // the number of pages in this file
    size_t data_offset = 0; // the number of bytes written to the buffer
    size_t request_byte = size * count; // the number of bytes user request to read
    size_t copy_byte = 0;
    void* page_data_addr = ((char*)PHYS_BASE) + ((page - mem_map) * PAGE_SIZE);

    /* get the number of pages in this file */
    memcpy(hd, page_data_addr, PAGE_HEADER_SIZE);
    if(unlikely(hd == NULL)) {printf("ERROR: write_to_buffer hd is NULL\n");}
    page_cnt = hd->PAGES;
    /* write the data in the file to user's buffer */
    if (unlikely(request_byte < PAGE_SIZE - PAGE_HEADER_SIZE))
    {
        copy_byte = request_byte;
    }
    else
    {
        copy_byte = PAGE_SIZE - PAGE_HEADER_SIZE;
    }

    memcpy(buffer, page_data_addr + PAGE_HEADER_SIZE, copy_byte); // write first page
    data_offset = PAGE_SIZE - PAGE_HEADER_SIZE;
    request_byte-=data_offset;
    for (unsigned int i = 1;i < page_cnt;i++)
    {
        page = page->next; // move to next page
        page_data_addr = ((char*)PHYS_BASE) + ((page - mem_map) * PAGE_SIZE);
        if (unlikely(request_byte < PAGE_SIZE))
        {
            copy_byte = request_byte;
        }
        else
        {
            copy_byte = PAGE_SIZE;
        }
        memcpy(buffer + data_offset, page_data_addr, copy_byte);
        data_offset+=copy_byte; // modify data offset
        request_byte-=PAGE_SIZE;
        if(request_byte == 0)
        {
            break;
        }
    }

    return data_offset / size;
}

size_t uread(void* buffer, size_t size, size_t count, uFILE* stream)
{
    size_t write_cnt;
    hash_entry* target_entry = hash_table_lookup(stream->path_name);

    if (target_entry != NULL) // if the page is in the hash table (which means it is in the LRU list
    {
        /* move the file to the head of LRU list */
        move_to_lru_head(&lru_list, target_entry->lru_entry_ptr);

        /* write the data into user's buffer */
        if (target_entry->lru_entry_ptr == NULL)
        {
            printf("ERROR: target_entry->lru_entry_ptr\n");
            return -1;
        }
        if (target_entry->lru_entry_ptr->page_ptr == NULL)
        {
            printf("ERROR: target_entry->lru_entry_ptr->page_ptr\n");
            return -1;
        }
        page* target_page = target_entry->lru_entry_ptr->page_ptr;
        write_cnt = write_to_buffer(buffer, size, count, target_page);
    }
    else
    {
        /*if the page is not in the page cache*/
        page* target_page = alloc_page();
        if (unlikely(!target_page))
        {
            printf("ERROR: target_page\n");
            return -1;
        } // failed to get a new page, return

        /*setting infomation of new page */
        target_page->path_name = stream->path_name;
        target_page->index = 0;
        target_page->flag |= PG_lru;

        read_pio(target_page, PHYS_BASE, mem_map);
        add_to_lru_head(&lru_list, target_page);

        /*write the data into user's buffer*/
        write_cnt = write_to_buffer(buffer, size, count, target_page);
    }

    return write_cnt;
}