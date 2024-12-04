#include "lru.h"

hash_entry* hash_table[CACHE_SIZE] = {NULL};

void add_to_lru_head(lru_cache* lru_list, page* pg)
{
    lru_entry* hd = (lru_entry*)umalloc_share(SHM_NAME, sizeof(lru_entry));
    hd->page_ptr = pg;

    /* set the data of new page */
    hd->page_ptr->flag |= PG_lru;

    /* modify the data of LRU list*/
    if (lru_list->head) {lru_list->head->prev = hd;}
    hd->next = lru_list->head;
    lru_list->head = hd;
    if (lru_list->tail == NULL) {lru_list->tail = hd;} // lru list is empty, so the tail is new lru entry
    hash_table_insert(hd);
}

void move_to_lru_head(lru_cache* lru_list, lru_entry* hd)
{
    if(hd->prev) {hd->prev->next = hd->next;} // modify previous lru entry's next
    else {return;} // this lru entry is already the head, return
    if(hd->next) {hd->next->prev = hd->prev;}
    else {lru_list->tail = hd->prev;} // this page is tail, so the previous page becomes the new tail

    hd->prev = NULL;
    hd->next = lru_list->head;
    lru_list->head = hd; // head of lru list change to this page
}

int remove_from_lru(lru_cache* lru_list, lru_entry* hd)
{
    if(unlikely(hash_table_remove(hd) == 1))
    {
        perror("ERROR: remove_from_lru not found in hash table");
        return 1;
    }

    /* Remove the page from the list */
    if (hd->prev) {hd->prev->next = hd->next;} // modify head's previous page
    else {lru_list->head = hd->next;} // this page is the head, so the next page becomes the new head
    if(hd->next) hd->next->prev = hd->prev;
    else {lru_list->tail = hd->prev;} // this page is tail, so the previous page becomes the new tail

    return 0;
}

unsigned int hash_function(char* path_name)
{
    unsigned int hash = 0;
    while (unlikely(*path_name))
    {
        hash = (hash * 31) + *path_name++;
    }
    return hash % CACHE_SIZE;
}

hash_entry* hash_table_lookup(char* path_name)
{
    const unsigned int hash_index = hash_function(path_name);
    hash_entry* entry = hash_table[hash_index];
    /*Check if the entry belongs to this page; if not, move on to the next entry*/
    while (entry)
    {
        if (strcmp(entry->lru_entry_ptr->page_ptr->path_name, path_name) == 0) // check if the entry belongs to this page
        {
            return entry;
        }
        entry = entry->next; // move on to the next entry
    }
    return NULL;
}

void hash_table_insert(lru_entry* hd)
{
    char* path_name = hd->page_ptr->path_name;
    const unsigned int hash_index = hash_function(path_name);
    hash_entry* new_entry = (hash_entry*)umalloc_share(SHM_NAME, sizeof(hash_entry));

    /*Set the information in the hash entry*/
    new_entry->lru_entry_ptr = hd;
    if (hash_table[hash_index] == NULL) {hash_table[hash_index] = new_entry;}
    else {hash_table[hash_index]->next = new_entry;}
}

int hash_table_remove(lru_entry* hd)
{
    char* path_name = hd->page_ptr->path_name;
    hash_entry* lookup_entry = hash_table_lookup(path_name);
    if (unlikely(lookup_entry == NULL))
    {
        return 1; // not found;
    }
    else
    {
        if(lookup_entry->next == NULL) {ufree(lookup_entry);}
        else
        {
            hash_entry* remove_hash_entry = lookup_entry;
            lookup_entry = lookup_entry->next;
            ufree(remove_hash_entry);
        }
    }
    return 0;
}