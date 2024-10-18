# Development Log

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