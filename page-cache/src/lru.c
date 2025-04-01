#include "lru.h"
#include "upage.h"

hash_entry* hash_table[CACHE_SIZE] = {NULL};

void add_to_lru_head(lru_cache* lru_list, page* pg)
{
    /* set the data of new page */
    pg->flag |= PG_lru;

    /* modify the data of LRU list*/
    if (lru_list->head) {lru_list->head->prev = pg;}
    pg->next = lru_list->head;
    lru_list->head = pg;
    if (lru_list->tail == NULL) {lru_list->tail = pg;} // lru list is empty, so the tail is new lru entry
    hash_table_insert(pg);
}

void move_to_lru_head(lru_cache* lru_list, page* pg)
{
    if(pg->prev) {pg->prev->next = pg->next;} // modify previous lru entry's next
    else {return;} // this lru entry is already the head, return
    if(pg->next) {pg->next->prev = pg->prev;}
    else {lru_list->tail = pg->prev;} // this page is tail, so the previous page becomes the new tail

    pg->prev = NULL;
    pg->next = lru_list->head;
    lru_list->head = pg; // head of lru list change to this page
}

int remove_from_lru(lru_cache* lru_list, page* pg)
{
    if(unlikely(hash_table_remove(pg) == 1))
    {
        perror("ERROR: remove_from_lru not found in hash table");
        return 1;
    }

    /* free all page of the path name */
    free_page(pg);
    /* Remove the page from the list */
    if (pg->prev) {pg->prev->next = pg->next;} // modify head's previous page
    else {lru_list->head = pg->next;} // this page is the head, so the next page becomes the new head
    if(pg->next) pg->next->prev = pg->prev;
    else {lru_list->tail = pg->prev;} // this page is tail, so the previous page becomes the new tail

    ufree(pg);

    return 0;
}

// void print_all_pages(page* head) {
//     if (head == NULL) {
//         printf("No pages to display. The list is empty.\n");
//         return;
//     }

//     page* current = head;
//     int count = 1;  // To track the number of pages in the list

//     while (current != NULL) {
//         printf("\n--- Page %d ---\n", count);
//         printf("Flag: %u\n", current->flag);
//         printf("Path Name: %s\n", current->path_name);
//         printf("Index: %u\n", current->index);
//         printf("Next Page: %p\n", current->next);

//         current = current->next;  // Move to the next page in the list
//         count++;
//     }
//     printf("\n");
// }

// void print_lru_entry(page* entry) {
//     if (entry == NULL) {
//         printf("LRU Entry is NULL\n");
//         return;
//     }
//     print_all_pages(entry->page_ptr);
//     printf("LRU Entry Info:\n");
//     printf("Page Pointer: %p\n", entry->page_ptr);
//     printf("Previous LRU Entry: %p\n", entry->prev);
//     printf("Next LRU Entry: %p\n", entry->next);
//     printf("\n");
// }

// void print_lru_cache(lru_cache* cache) {
//     if (cache == NULL) {
//         printf("LRU Cache is NULL\n");
//         return;
//     }

//     printf("LRU Cache Info:\n");

//     page* current_entry = cache->head;
//     if (current_entry == NULL) {
//         printf("LRU Cache is empty\n");
//         return;
//     }

//     int count = 1;  // To keep track of the number of entries in the list
//     while (current_entry != NULL) {
//         printf("\n--- Entry %d ---\n", count);
//         print_lru_entry(current_entry);
//         current_entry = current_entry->next;
//         count++;
//     }
//     printf("\n");
// }

unsigned int hash_function(char* path_name, unsigned int index)
{
    unsigned int hash = 0;
    while (unlikely(*path_name))
    {
        hash = (hash * 31) + *path_name++;
    }
    hash = (hash * 31) + index;
    return hash % CACHE_SIZE;
}

hash_entry* hash_table_lookup(char* path_name, unsigned int index)
{
    const unsigned int hash_index = hash_function(path_name, index);
    hash_entry* entry = hash_table[hash_index];
    /*Check if the entry belongs to this page; if not, move on to the next entry*/
    while (entry)
    {
        if (strcmp(entry->page_ptr->path_name, path_name) == 0 && (entry->page_ptr->index == index)) // check if the entry belongs to this page
        {
            return entry;
        }
        entry = entry->next; // move on to the next entry
    }
    return NULL;
}

void hash_table_insert(page* pg)
{
    char* path_name = pg->path_name;
    const unsigned int hash_index = hash_function(path_name, pg->index);
    hash_entry* new_entry = (hash_entry*)umalloc_dma(sizeof(hash_entry));

    /*Set the information in the hash entry*/
    new_entry->page_ptr = pg;
    new_entry->next = NULL;
    if (hash_table[hash_index] == NULL)
    {
        hash_table[hash_index] = new_entry;
    }
    else
    {
        new_entry->next = hash_table[hash_index];
        hash_table[hash_index] = new_entry;
    }
}

int hash_table_remove(page* pg)
{
    char* path_name = pg->path_name;
    const unsigned int hash_index = hash_function(path_name, pg->index);
    hash_entry* current = hash_table[hash_index];
    hash_entry* prev = NULL;
    bool find = false;

    if (unlikely(current == NULL))
    {
        return 1; // not found;
    }
    else
    {
        /* Check if the entry must be removed; if not, proceed to the next entry */
        while (current)
        {
            if (strcmp(current->page_ptr->path_name, path_name) == 0 && (current->page_ptr->index == pg->index)) // If the current entry is the target entry
            {
                if (prev == NULL)
                {
                    hash_table[hash_index] = current->next;
                }
                else
                {
                    prev->next = current->next;
                }
                find = true; // find the hash entry
                free_dma_buffer(current);
                break;
            }
            prev = current;
            current = current->next; // move on to the next entry
        }
    }

    if(unlikely(find == false))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}