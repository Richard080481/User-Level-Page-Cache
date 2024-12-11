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
struct page_free_list free_list = {NULL, 0};

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
    unsigned int page_cnt = 0;
    memcpy(&page_cnt, page_data_addr, PAGE_HEADER_SIZE);

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
}

uFILE* uopen(const char* filename, const char* mode)
{
    uFILE* puf = umalloc_dma(sizeof(uFILE));
    puf->path_name = filename;
    if (strchr(mode, 'r')) {puf->mode |= U_OREAD;}
    if (strchr(mode, 'w')) {puf->mode |= U_OWRITE;}
    if (strchr(mode, '+')) {puf->mode |= U_REMOVE;}
    return puf;
}

int uclose(uFILE* stream)
{
    stream->path_name = "\0";
    stream->mode = U_INVALID;
    free(stream);
    return 0;
}

size_t uwrite(const void* buffer, size_t size, size_t count, uFILE* stream)
{
    if ((stream->mode) & U_OREAD == 0) // if U_OREAD is 0, this file cannot be written to; return 0
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
            memcpy(page_data_addr + PAGE_HEADER_SIZE, buffer, copy_len);

            data_offset += (copy_len + PAGE_HEADER_SIZE);
            if(unlikely(DATA_LEN + PAGE_HEADER_SIZE <= PAGE_SIZE))
            {
                add_to_lru_head(&lru_list, new_page);
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

        add_to_lru_head(&lru_list, new_page);
        data_offset += copy_len;
        index++;
    }

    return count;
}

// for uread
void write_to_buffer(void* buffer, size_t size, size_t count, page* page)
{
    unsigned int page_cnt = 0; // the number of pages in this file
    unsigned int data_offset = 0; // the number of bytes written to the buffer
    void* page_data_addr = ((char*)PHYS_BASE) + ((page - mem_map) * PAGE_SIZE);

    /* get the number of pages in this file */
    memcpy(&page_cnt, page_data_addr, PAGE_HEADER_SIZE);

    /* write the data in the file to user's buffer */
    memcpy(buffer, page_data_addr + PAGE_HEADER_SIZE, PAGE_SIZE - PAGE_HEADER_SIZE); // write first page
    data_offset = PAGE_SIZE - PAGE_HEADER_SIZE;
    for (unsigned int i = 1;i < page_cnt;i++)
    {
        page = page->next; // move to next page
        page_data_addr = ((char*)PHYS_BASE) + ((page - mem_map) * PAGE_SIZE);
        memcpy(buffer + data_offset, page_data_addr, PAGE_SIZE);
        data_offset+=PAGE_SIZE; // modify data offset
    }
}

size_t uread(void* buffer, size_t size, size_t count, uFILE* stream)
{
    hash_entry* target_entry = hash_table_lookup(stream->path_name);

    if (target_entry != NULL) // if the page is in the hash table (which means it is in the LRU list)
    {
        /* move the file to the head of LRU list */
        move_to_lru_head(&lru_list, target_entry->lru_entry_ptr);

        /* write the data into user's buffer */
        page* target_page = target_entry->lru_entry_ptr->page_ptr;
        write_to_buffer(buffer, size, count, target_page);
    }
    else
    {
        /*if the page is not in the page cache*/
        page* target_page = alloc_page();
        if (unlikely(!target_page)) {return -1;} // failed to get a new page, return

        /*setting infomation of new page */
        target_page->path_name = stream->path_name;
        target_page->index = 0;
        target_page->flag |= PG_lru;

        read_pio(target_page, PHYS_BASE, mem_map);
        add_to_lru_head(&lru_list, target_page);

        /*write the data into user's buffer*/
        write_to_buffer(buffer, size, count, target_page);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    init_page_cache();
    int STRING_LENGTH = 13000;
    char *test_string = (char *)malloc(STRING_LENGTH + 1);
    if (test_string == NULL)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < STRING_LENGTH; i++)
    {
        test_string[i] = '0' + (i % 10);
    }
    test_string[STRING_LENGTH] = '\0';

    uFILE *file = uopen("test_output.txt", "w");
    if (file == NULL)
    {
        printf("Failed to open file for writing.\n");
        free(test_string);
        return 1;
    }
    fprintf(file, "%s", test_string);
    uclose(file);
    printf("String written to file successfully.\n");

    char *read_string = (char *)malloc(STRING_LENGTH + 1);
    if (read_string == NULL)
    {
        printf("Memory allocation failed.\n");
        free(test_string);
        return 1;
    }
    file = uopen("test_output.txt", "r");
    if (file == NULL)
    {
        printf("Failed to open file for reading.\n");
        free(test_string);
        free(read_string);
        return 1;
    }
    uread(read_string, 1, STRING_LENGTH, file);
    read_string[STRING_LENGTH] = '\0';
    uclose(file);

    if (strcmp(test_string, read_string) == 0)
    {
        printf("Read and write are correct.\n");
    } else
    {
        printf("Mismatch in read and write.\n");
    }

    free(test_string);
    free(read_string);

    exit_page_cache();
    return 0;
}