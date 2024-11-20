#include "lru.h"
#include "types.h"
#include "upage.h"

void add_to_lru_head(lru_cache lru_list, page* p)
{
    p->flag |= PG_lru;

    p->next = lru_list.head;
    if (lru_list.head) {lru_list.head->prev = p;}
    lru_list.head = p;
    if (lru_list.tail == NULL) {lru_list.tail = p;}
}