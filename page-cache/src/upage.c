#include "upage.h"
#include "umalloc.h"
#include "cache_api.h"
#include "spdk.h"
#include <spdk/env.h>
#include <spdk/memory.h>
#include <string.h>
#include <error.h>
#include <sys/mman.h>
#include <pthread.h>

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

    mem_map = (page*)umalloc_dma(CACHE_SIZE * (size_t)sizeof(page)); // allocate space for struct PAGE
    PHYS_BASE =  umalloc_dma(CACHE_SIZE * PAGE_SIZE);

    /* lock page cache into memory */
    if (mlock(mem_map, CACHE_SIZE * (size_t)sizeof(page)) != 0) { perror("mlock fail\n"); }
    if (mlock(PHYS_BASE, CACHE_SIZE * PAGE_SIZE) != 0) { perror("mlock fail\n"); }

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
    /* unlock page cache into memory */
    if (munlock(mem_map, CACHE_SIZE * (size_t)sizeof(page)) != 0) { perror("munlock fail\n"); }
    if (munlock(PHYS_BASE, CACHE_SIZE * PAGE_SIZE) != 0) { perror("minlock fail\n"); }

    free_dma_buffer(mem_map);
    free_dma_buffer(PHYS_BASE);
    exit_ssd_cache();
    return 0;
}

void* write_back_thread(void* arg)
{   
    printf("do write back\n");
    for (int i = 0;i < (CACHE_SIZE >> 1);i++) // Write back some pages in the page cache.
    {
        write_pio(lru_list.tail, PHYS_BASE, mem_map); // write the page into ssd
        remove_from_lru(&lru_list, lru_list.tail); // remove lru entry (remove all pages of the file)
    }
    return NULL;
}

page* alloc_page(void)
{
    if (free_list.nr_free == 0) // the number of free page is zero, do write-back
    {
        for (int i = 0;i < (CACHE_SIZE >> 1);i++) // Write back some pages in the page cache.
        {
            write_pio(lru_list.tail, PHYS_BASE, mem_map); // write the page into ssd
            remove_from_lru(&lru_list, lru_list.tail); // remove lru entry (remove all pages of the file)
        }
    }

    /* allocate a new page from the head of free list */
    page* new_page = free_list.head;
    free_list.head = free_list.head->next;
    free_list.nr_free--;

    new_page->flag = 0;
    new_page->index = 0;
    new_page->next = NULL;
    new_page->prev = NULL;
    new_page->path_name = NULL;

    if(unlikely(!new_page))
    {
        printf("Error: allocate a page failed\n");
    }
    return new_page;
}

void free_page(page* target_page)
{
    target_page->next = free_list.head;
    free_list.head = target_page;
    
    /* clear all data in the page */
    target_page->path_name = NULL;
    target_page->index = 0;
    target_page->flag = 0;
    target_page->prev = NULL;

    /* move to next page */
    free_list.nr_free++;

    return;
}

uFILE* uopen(char* filename, const char* mode)
{
    uFILE* puf = umalloc_dma(sizeof(uFILE));
    puf->path_name = filename;
    puf->mode = 0x00;
    puf->io_offset = 0;
    if (strchr(mode, 'r')) {puf->mode |= U_OREAD;}
    if (strchr(mode, 'w')) {puf->mode |= U_OWRITE;}
    if (strchr(mode, '+')) {puf->mode |= U_REMOVE;}
    return puf;
}

int uclose(uFILE* stream)
{
    // hash_entry* target_entry = hash_table_lookup(stream->path_name, stream->io_offset >> 12); // only for 4kb
    // write_pio(target_entry->page_ptr, PHYS_BASE, mem_map); // write the page into ssd
    // remove_from_lru(&lru_list, target_entry->page_ptr); // remove lru entry (remove all pages of the file)
    stream->path_name = "\0";
    stream->mode = U_INVALID;
    stream->io_offset = 0;
    free_dma_buffer(stream);
    return 0;
}

void convert_unsigned_int_to_string(unsigned int num, char* len)
{
    unsigned int index = 0;
    do
    {
        len[index] = '0' + (num % 10);
        num /= 10;
        index++;
    } while (num > 0);

    while (index < PAGE_HEADER_SIZE)
    {
        len[index++] = '0';
    }
}

unsigned int convert_string_to_unsigned_int(const char* buffer)
{
    unsigned int result = 0;
    unsigned int multiplier = 1;

    for (int i = 0; i < PAGE_HEADER_SIZE && buffer[i] != '0'; i++)
    {
        result += (buffer[i] - '0') * multiplier;
        multiplier *= 10;
    }

    return result;
}

size_t uwrite(void* buffer, size_t size, size_t count, uFILE* stream)
{
    if (((stream->mode) & U_OWRITE) == 0) // if U_OREAD is 0, this file cannot be written to; return 0
    {
        return 0;
    }

    // struct timespec tt1, tt2;
    // clock_gettime(CLOCK_REALTIME, &tt1);

    char* path_name = stream->path_name;
    const unsigned int DATA_LEN = size * count;
    const unsigned int DATA_PAGES = (DATA_LEN + PAGE_SIZE - 1) / PAGE_SIZE ; // ceiling division to calculate the pages
    unsigned int data_offset = 0;
    unsigned int index = (stream->io_offset) >> 12;
    unsigned int offset_in_page = 0;// unsigned int offset_in_page = (stream->io_offset) & (PAGE_SIZE - 1);
    bool isLastPage = false;
    bool isFirstPage = true;

    /* if the size of file is bigger than page cache size */
    if (DATA_PAGES > CACHE_SIZE)
    {
        void* first_page = umalloc_dma(PAGE_SIZE);
        void* last_page = umalloc_dma(PAGE_SIZE);

        /* creat pio */
        operate operation = WRITE;
        struct pio* head;

        /* write first page */
        memcpy(first_page, buffer, PAGE_SIZE - offset_in_page);
        data_offset = PAGE_SIZE - offset_in_page;
        head = create_pio(path_name, 0, index, operation, first_page, 1);
        submit_pio(head);
        free_pio(head);
        ufree(first_page);
        index++;

        while (DATA_LEN - data_offset > PAGE_SIZE)
        {
            head = create_pio(path_name, 0, index, operation, buffer + data_offset, 1);
            submit_pio(head);
            free_pio(head);
            index++;
            data_offset += PAGE_SIZE;
        }

        /* write last page */
        memcpy(last_page, buffer + data_offset, DATA_LEN - data_offset);
        head = create_pio(path_name, 0, index, operation, last_page, 1);
        submit_pio(head);
        free_pio(head);
        ufree(last_page);

        return data_offset / size;
    }

    while (isLastPage == false) // The data has not yet been written
    {
        unsigned int copy_len;
        page* new_page = alloc_page();
        void* page_data_addr = ((char*)PHYS_BASE) + ((new_page - mem_map) << 12);

        if (unlikely(!new_page)) {return -1;} // failed to get a new page, return

        /* setting infomation of new page */
        new_page->flag |= PG_dirty;
        new_page->index = index;
        new_page->path_name = path_name;
        index++;

        /* Write the data into new page */
        if (unlikely(isFirstPage == true))
        {
            // struct timespec tt1, tt2;
            // clock_gettime(CLOCK_REALTIME, &tt1);
            
            isFirstPage = false;

            // If the first page is also last page
            if (unlikely(DATA_LEN <= PAGE_SIZE - offset_in_page)) {isLastPage = true;}

            copy_len = (DATA_LEN <= PAGE_SIZE - offset_in_page) ? DATA_LEN : PAGE_SIZE - offset_in_page;

            // Copy the data chunk into the package
            memcpy(page_data_addr + offset_in_page, buffer, copy_len);
            data_offset = copy_len;

            // clock_gettime(CLOCK_REALTIME, &tt2);
            // printf("memcpy consumes %ld nanoseconds!\n", tt2.tv_nsec - tt1.tv_nsec);

            add_to_lru_head(&lru_list, new_page);

            if(DATA_LEN - data_offset == 0) {break;}
            else {continue;}
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
    }

    // clock_gettime(CLOCK_REALTIME, &tt2);
    // printf("write consumes %ld nanoseconds!\n", tt2.tv_nsec - tt1.tv_nsec);

    return data_offset / size;
}

size_t uread(void* buffer, size_t size, size_t count, uFILE* stream)
{
    unsigned int request_byte = size * count;
    unsigned int request_pages = (request_byte + PAGE_SIZE - 1) / PAGE_SIZE ;
    unsigned int copy_len = 0;
    unsigned int buffer_offset = 0;
    char* path_name = stream->path_name;
    void* page_data_addr;
    page* target_page;
    bool isFirstPage;
    unsigned int index = (stream->io_offset) >> 12;
    unsigned int offset_in_page = 0;// (stream->io_offset) & (PAGE_SIZE - 1)

    if (unlikely(request_pages > CACHE_SIZE)) // file is bigger than cache size
    {
        void* first_page = umalloc_dma(PAGE_SIZE);

        /* read first page */
        operate operation = READ;
        struct pio* head = create_pio(path_name, 0, index, operation, first_page, 1); // creat pio for the first page
        submit_pio(head);
        free_pio(head);
        memcpy(buffer, first_page + offset_in_page, PAGE_SIZE - offset_in_page);
        ufree(first_page);
        index++;
        buffer_offset += (PAGE_SIZE - offset_in_page);
        request_byte -= buffer_offset;

        while (request_byte >= PAGE_SIZE)
        {
            head = create_pio(path_name, 0, index, operation, buffer + buffer_offset, 1); // creat pio for the first page
            submit_pio(head);
            free_pio(head);
            index++;
            buffer_offset += PAGE_SIZE;
            request_byte -= PAGE_SIZE;
        }

        if (request_byte)
        {
            void* last_page = umalloc_dma(PAGE_SIZE);
            head = create_pio(path_name, 0, index, operation, last_page, 1); // creat pio for the first page
            submit_pio(head);
            free_pio(head);
            memcpy(buffer + buffer_offset, last_page, request_byte);
            ufree(last_page);
            buffer_offset += request_byte;
            request_byte = 0;
        }

        return buffer_offset / size;
    }

    while (request_byte > buffer_offset) {

        if (unlikely(isFirstPage == true))
        {
            isFirstPage = false;
            copy_len = (request_byte <= PAGE_SIZE - offset_in_page) ? request_byte : PAGE_SIZE - offset_in_page;
        }
        else if((unlikely(request_byte - copy_len <= PAGE_SIZE)))
        {
            copy_len = (request_byte - copy_len);
        }
        else
        {
            copy_len = PAGE_SIZE;
        }

        hash_entry* target_entry = hash_table_lookup(stream->path_name, index);

        if (target_entry != NULL) // if the page is in the hash table (which means it is in the LRU list
        {
            // printf("in page cache\n");
            /* move the file to the head of LRU list */
            move_to_lru_head(&lru_list, target_entry->page_ptr);
            page_data_addr = ((char*)PHYS_BASE) + ((target_entry->page_ptr - mem_map) * PAGE_SIZE);
            memcpy(buffer + buffer_offset, page_data_addr + offset_in_page, copy_len);
        }
        else
        {
            // printf("not in page cache\n");
            target_page = alloc_page();
            target_page->flag |= PG_lru;
            target_page->index = index;
            target_page->path_name = stream->path_name;

            read_pio(target_page, PHYS_BASE, mem_map);
            add_to_lru_head(&lru_list, target_page);
            /*write the data into user's buffer*/
            page_data_addr = ((char*)PHYS_BASE) + ((target_page - mem_map) * PAGE_SIZE);
            memcpy(buffer + buffer_offset, page_data_addr + offset_in_page, copy_len);
        }
        index++;
        buffer_offset += copy_len;
        offset_in_page = 0;
    }

    return buffer_offset / size;
}

void useek(uFILE* stream, unsigned long long int offset)
{
    stream->io_offset = offset;
}