# Development Log

## Date: 2024-11-3

### Today's Progress
- Implemeant page_cache_write, move_to_lru_head, alloc_page.
- Modify the struct of page.

### Issues Encountered
- The way to record the page index.
- The allocate page function needs to call the function create_pio, but create_pio requires the page index as a parameter.
  
### Plans for Tomorrow
- Implement page index recording.
- Implement read page cache.

## Date: 2024-10-23

### Today's Progress
- Fix dependency problem while executing make in page-cache.
- Implement init_page_cache() and exit_page_cache().
- Add struct page.
- Add a .gitignore file.

### Plans for Tomorrow
- Implement the functions in upage.h.
- Add a prototype of read and write page cache.
- Implement read and write page cache.

## Date: 2024-10-18

### Today's Progress
- Add Makefile for page-cache and make it call the dm-cache makefile.
- Add a prototype of init_page_cache.

### Issues Encountered
- Encountered dependency problem in SPDK while executing make in page-cache.

### Plans for Tomorrow
- Fix dependency problem to successfully execute Makefile.
- When dependecy problem fix, implement init_page_cache().

### Reflections and Learnings
- REMEMBER!!! to use dma_malloc() in spdk.
