#include "lru.h"
#include "types.h"
#include "upage.h"

void add_to_lru_head(lru_cache* lru_list, page* p)
{

    /* set the data of new LRU entry */
    lru_entry* new_lru_entry;
    new_lru_entry->page_ptr = p;
    new_lru_entry->next = lru_list->head;
    new_lru_entry->prev = NULL;

    /* set the data of new page */
    p->flag |= PG_lru;
    p->lru_ptr = new_lru_entry;

    /* modify the data of LRU list*/
    if (lru_list->head) {lru_list->head->prev = new_lru_entry;}
    lru_list->head = new_lru_entry;
    if (lru_list->tail == NULL) {lru_list->tail = new_lru_entry;} // lru list is empty, so the tail is new lru entry

    lru_list->nr_pages++;
}

void move_to_lru_head(lru_cache* lru_list, page* p)
{
    lru_entry* target_lru_entry = p->lru_ptr;

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