#ifndef UPIO_H
#define UPIO_H

#include "upage.h"
#include "umalloc.h"
#include "lru.h"

/**
 * @brief send pio(write) to dm-cache
 * @return No return value
 */
void write_pio(page* p, void* PHYS_BASE, page* mem_map);
/**
 * @brief send pio(read) to dm-cache
 * @return No return value
 */
void read_pio(page* p, void* PHYS_BASE, page* mem_map);
#endif