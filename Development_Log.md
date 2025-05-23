# Development Log

## Date: 2025-01-22 Bonnie

### Today's Progress
- Handle the case where the file size exceeds the cache size
- Fix the bug where the page is not returned to the free list after the file is removed from the LRU
- Modify ufree() to use SPDK's function

### Plans for Tomorrow
- Use fio to test the performance
- Modify uwrite and uread to support other data types
- Add warning for incorrect argument (uwrite : sizeof(char), uread : sizeof(int), count)

## Date: 2024-12-12 Richard & Bonnie

### Today's Progress
- Fix the way to get page count
- Fix the page index problem
- Fix uwrite data offset

### Plans for Tomorrow
- Check what will happen went page_cache is too small
- Modify uwrite and uread to support other data types
- Add warning for incorrect argument (uwrite : sizeof(char), uread : sizeof(int), count)

## Date: 2024-12-11 Richard & Bonnie

### Today's Progress
- Fix write_to_buffer
- separate main to app.c
- Fix the problem that add every page to lru_cache

### Plans for Tomorrow
- Modify the way to read page_cnt
- Check the page index problem
- Check what will happen went page_cache is too small

## Date: 2024-12-05 Richard & Bonnie

### Today's Progress
- Add data structure of ufile
- Add uopen and uclose
- Modify parameters of uwrite and uread to match POSIX semantics
- Fix segmentation fault

### Plans for Tomorrow
- Modify uwrite to add '\0' in the of of each page
- Fix uread and write_to_buffer
- Fix uwrite return value

## Date: 2024-12-04 Richard & Bonnie

### Today's Progress
- Add utypes.h
- Fixed compile error

### Plans for Tomorrow
- Think about how to implement umalloc
- Fix segmentation fault

## Date: 2024-11-28 Richard & Bonnie

### Today's Progress
- Fixed alloc_page and free_page
- Fixed parameter passing
- implement lru and hash table

### Plans for Tomorrow
- Fix compiler error
- Design link_page_cache and unlink_page_cache function.
- Think about multiple process and share memory problem.

## Date: 2024-11-20 Richard & Bonnie

### Today's Progress
- Implement page_cache_write with header function.
- Create umalloc.h for spdk_zmalloc()
- Set up header struct.
- implement likely() and unlikely() for compiler

### Plans for Tomorrow
- Design link_page_cache and unlink_page_cache function.
- Read implementation
- hash implementation (only path_name in hash table)
- file header struct in lru implementation
- lru operation implementation
- pio append implementation
- write implementation accommodate with lru
- separate different function into different file and header
- add more comment on source code
- add comment on header file of *.h

### Plans for Tomorrow
- Add total file length in first page.
- Make sure there is only one page cache in the operating system.

## Date: 2024-11-14 Richard & Bonnie

### Today's Progress
- Implement read_pio and write_pio


### Plans for Tomorrow
- Add total file length in first page.
- Make sure there is only one page cache in the operating system.

## Date: 2024-11-9 Richard

### Today's Progress
- Add make cleanall to delete *.o and *.d file.

## Date: 2024-11-3 Bonnie

### Today's Progress
- Implement page_cache_write, move_to_lru_head, alloc_page.
- Modify the struct of page.

### Issues Encountered
- The way to record the page index.
- The allocate page function needs to call the function create_pio, but create_pio requires the page index as a parameter.

### Plans for Tomorrow
- Implement page index recording.
- Implement read page cache.

## Date: 2024-10-23 Bonnie

### Today's Progress
- Fix dependency problem while executing make in page-cache.
- Implement init_page_cache() and exit_page_cache().
- Add struct page.
- Add a .gitignore file.

### Plans for Tomorrow
- Implement the functions in upage.h.
- Add a prototype of read and write page cache.
- Implement read and write page cache.

## Date: 2024-10-18 Richard

### Today's Progress
- Add Makefile for page-cache and make it call the dm-cache makefile.
- Add a prototype of init_page_cache.

### Issues Encountered
- Encountered dependency problem in SPDK while executing make in page-cache.

### Plans for Tomorrow
- Fix dependency problem to successfully execute Makefile.
- When dependency problem fix, implement init_page_cache().

### Reflections and Learnings
- REMEMBER!!! to use dma_malloc() in spdk.
