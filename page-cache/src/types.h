#ifndef PAGE_STRUCT
#define PAGE_STRUCT
typedef struct Header
{
    unsigned int PAGES;         // How many pages in the file header.
} header;

typedef struct PAGE
{
    unsigned flag;
    char* path_name;
    unsigned int index;
    struct PAGE* prev;
    struct PAGE* next;
}page;

enum pageflags
{
    PG_locked = 0x01,
    PG_dirty = 0x02,
    PG_lru = 0x04,
};

typedef struct lru_cache
{
    page* head;
    page* tail;
    int nr_pages;
} lru_cache;

typedef struct page_free_list
{
    page* head;
    int nr_free;
} page_free_list;
#endif