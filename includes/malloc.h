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
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void show_alloc_mem();

#endif

//
// ================ ALLOCATOR STATE ================
//
// HEAP #0 @ 0x0000000102000000
//   size      : 4096 bytes
//   used      : 1824 bytes
//   free      : 2272 bytes
//   blocks    : 5
//
//   [0x102000000] HDR size=64  ALLOC
//   [0x102000040] BLK size=128 ALLOC
//   [0x1020000c0] FTR size=128
//
//   [0x1020000c8] HDR size=256 FREE
//   [0x1020001c8] FREE NEXT=0x102000300 PREV=0x102000080
//   [0x1020002c8] FTR size=256
//
//   [0x1020002d0] HDR size=512 ALLOC
//   [0x1020004d0] BLK size=512 ALLOC
//   [0x1020006d0] FTR size=512
//
//   [0x1020006d8] HEAP FOOTER next_heap=0x102100000
//
// --------------------------------------------------
//
// HEAP #1 @ 0x0000000102100000
//   size      : 8192 bytes
//   used      : 1024 bytes
//   free      : 7168 bytes
//   blocks    : 2
//
//   [0x102100000] HDR size=1024 ALLOC
//   [0x102100400] BLK size=1024 ALLOC
//   [0x102100800] FTR size=1024
//
//   [0x102100808] HEAP FOOTER next_heap=NULL
//
// ==================================================
// TOTAL HEAP MEMORY:
//   allocated : 2848 bytes
//   free      : 9440 bytes
//   heaps     : 2
// ==================================================
