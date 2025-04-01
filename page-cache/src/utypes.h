#ifndef UDATASTRUCTURE_H
#define UDATASTRUCTURE_H

#define SHM_NAME "/myShm"

#define ENABLE_BRANCH_HINTS
#ifdef ENABLE_BRANCH_HINTS
    // Define likely and unlikely macros
    #define likely(x)   __builtin_expect(!!(x), 1)
    #define unlikely(x) __builtin_expect(!!(x), 0)
#else
    // When branch hints are disabled, likely/unlikely do nothing
    #define likely(x)   (x)
    #define unlikely(x) (x)
#endif

typedef struct PAGE
{
    unsigned flag;
    char* path_name;
    unsigned int index; // 0 index
    struct PAGE* prev;
    struct PAGE* next;
}page;

enum pageflags
{
    PG_locked = 0x01,
    PG_dirty = 0x02,
    PG_lru = 0x04,
};

typedef struct lru_entry
{
    page* page_ptr;
    struct lru_entry* prev;
    struct lru_entry* next;
} lru_entry;

typedef struct hash_entry
{
    page* page_ptr;
    struct hash_entry* next;
} hash_entry;

typedef struct lru_cache
{
    page* head;
    page* tail;
} lru_cache;

typedef struct Header
{
    unsigned int PAGES;         // How many pages in the file header.
} header;

enum uFILE_flags
{
    U_INVALID = 0x00,
    U_OREAD = 0x01,
    U_OWRITE = 0x02,
    U_REMOVE = 0x04,
};

typedef struct uFILE
{
    char* path_name;
    unsigned int mode;
    unsigned int io_offset;
} uFILE;

#endif