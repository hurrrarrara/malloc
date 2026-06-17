#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>

/* clang-format off
 * 
 * memory block layout:
 * | header -> [size:63, allocation status bit:1]
 * | payload
 * | footer -> [size:63, allocation status bit:1]
 *
 * free memory block layout:
 * | header -> [size:63, allocation status bit:1]
 * | next free block
 * | prev free block
 * | footer -> [size:63, allocation status bit:1]
 *
 * heap layout:
 * | header -> [size:63, allocation status bit (always allocated):1]
 * | payload
 * | footer -> [next heap:63, allocation status bit (always allocated):1]
 *
 */

void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);
void show_alloc_mem();

#endif
