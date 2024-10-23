# Development Log

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
