#include "lru.h"
#include "upage.h"


void add_to_lru_head(lru_cache* lru_list, lru_entry* hd)
{
    /* set the data of new page */
    hd->page_ptr->flag |= PG_lru;

    /* modify the data of LRU list*/
    if (lru_list->head) {lru_list->head->prev = hd;}
    lru_list->head = hd;
    if (lru_list->tail == NULL) {lru_list->tail = hd;} // lru list is empty, so the tail is new lru entry

    lru_list->nr_pages++;
}

void move_to_lru_head(lru_cache* lru_list, lru_entry* hd)
{
    lru_entry* target_lru_entry = hd->lru_ptr;

    if(target_lru_entry->prev) target_lru_entry->prev->next = target_lru_entry->next; // modify previous lru entry's next
    else {return;} // this lru entry is already the head, return

    if(target_lru_entry->next) target_lru_entry->next->prev = target_lru_entry->prev;
    else {lru_list->tail = target_lru_entry;} // this page is tail, so the previous page becomes the new tail

    target_lru_entry->prev = NULL;
    target_lru_entry->next = lru_list->head;

    if (lru_list->head) {lru_list->head->prev = p;} // modify head's previous page

    lru_list->head = target_lru_entry; // head of lru list change to this page

}

void remove_from_lru(lru_cache* lru_list, page* p)
{
    lru_entry* target_lru_entry = p->lru_ptr;

    /* Remove the page from the list */
    if (target_lru_entry->prev) {target_lru_entry->prev->next = target_lru_entry->next;} // modify head's previous page
    else {lru_list->head = target_lru_entry->next;} // this page is the head, so the next page becomes the new head

    if(target_lru_entry->next) target_lru_entry->next->prev = target_lru_entry->prev;
    else {lru_list->tail = p->prev;} // this page is tail, so the previous page becomes the new tail

    /* clear the LRU flag */
    p->flag &= ~PG_lru;
    p->lru_ptr = NULL;

}


unsigned int hash_function(char* path_name)
{
    unsigned int hash = 0;
    while (*path_name)
    {
        hash = (hash * 31) + *path_name++;
    }
    return hash % CACHE_SIZE;
}

page* hash_table_lookup(char* path_name)
{
    unsigned int hash_index = hash_function(path_name);
    hash_entry* entry = hash_table[hash_index];

    /*Check if the entry belongs to this page; if not, move on to the next entry*/
    while (entry)
    {
        if (strcmp(entry->path_name, path_name) == 0) // check if the entry belongs to this page
        {
            return entry->page_ptr;
        }
        entry = entry->next; // move on to the next entry
    }

    return NULL;
}

void hash_table_insert(char* path_name, page* page_ptr)
{
    unsigned int hash_index = hash_function(path_name);
    hash_entry* new_entry = (hash_entry*)malloc(sizeof(hash_entry));

    /*Set the information in the hash entry*/
    new_entry->path_name = strdup(path_name);
    new_entry->page_ptr = page_ptr;
    new_entry->next = hash_table[hash_index];
    hash_table[hash_index] = new_entry;
}

void hash_table_remove(char* path_name)
{
    unsigned int hash_index = hash_function(path_name);
    hash_entry* entry = hash_table[hash_index];
    hash_entry* prev = NULL;

    /* Check if the entry belongs to this page; if not, move on to the next entry */
    while (entry)
    {
        if (strcmp(entry->path_name, path_name) == 0) // the entry belongs to this page
        {
            if (prev != NULL) // if previous page is not NULL
            {
                prev->next = entry->next;
            }
            else // if previous page is NULL
            {
                hash_table[hash_index] = entry->next;
            }
            free(entry);
            return;
        }
        /* move on to the next entry */
        prev = entry;
        entry = entry->next;
    }
}