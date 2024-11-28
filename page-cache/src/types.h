#ifndef PAGE_STRUCT
#define PAGE_STRUCT
typedef struct Header
{
    unsigned int PAGES;         // How many pages in the file header.
} header;

typedef struct lru_entry lru_entry;

typedef struct PAGE
{
    unsigned flag;
    char* path_name;
    unsigned int index;
    struct PAGE* prev;
    struct PAGE* next;
    lru_entry* lru_ptr;
}page;

typedef struct lru_entry
{
    page* page_ptr;
    struct lru_entry* prev;
    struct lru_entry* next;
} lru_entry;

typedef struct lru_cache
{
    lru_entry* head;
    lru_entry* tail;
    int nr_pages;
} lru_cache;

enum pageflags
{
    PG_locked = 0x01,
    PG_dirty = 0x02,
    PG_lru = 0x04,
};

typedef struct page_free_list
{
    page* head;
    int nr_free;
} page_free_list;

typedef struct hash_entry
{
    char* path_name;
    page* page_ptr;
    struct hash_entry* next;
} hash_entry;
#endif