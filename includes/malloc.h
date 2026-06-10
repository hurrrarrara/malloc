#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>

typedef enum e_class_type {
	TINY,
	SMALL,
	LARGE
} t_class_type;

typedef struct s_heap {
	struct s_heap *prev;
	struct s_heap *next;
	void*	free_list;
	void*	head;
	t_class_type type;
} t_heap;

typedef union s_header {
	unsigned long mask;
	struct {
		unsigned long size : 63;
		unsigned long alloc : 1;
	};
} t_header;

typedef t_header t_footer;

typedef struct s_block_pointer {
	void* prev_free;
	void* next_free;
} t_block_pointer;

/*
                                 Format of an allocated memory block
    +---------------------------------------------------------------------------------------+
    |                                     64-bits wide                                      |
    +---------------------------------------------------------------------------------------+

    +-----------------------------------------------------------------------------+---------+ <- header
    |                                  block_size                                 |  alloc  |
    |                           (1 LSB implicitly 0)                              |   (1)   |
    |                                    64 bits                                  |  1 bit  |
    +-----------------------------------------------------------------------------+---------+ <- (aligned)
    |                                                                                       |
    |                                 Payload and Padding                                   |
    |                                   (N Memory Rows)                                     |
    |                                                                                       |
    |                                                                                       |
    +-----------------------------------------------------------------------------+---------+ <- footer
    |                                  block_size                                 |  alloc  |
    |                           (1 LSB implicitly 0)                              |   (1)   |
    |                                    64 bits                                  |  1 bit  |
    +-----------------------------------------------------------------------------+---------+


                                     Format of a free memory block
    +-----------------------------------------------------------------------------+---------+ <- header
    |                                  block_size                                 |  alloc  |
    |                           (1 LSB implicitly 0)                              |   (0)   |
    |                                    64 bits                                  |  1 bit  |
    +-----------------------------------------------------------------------------+---------+ <- (aligned)
    |                                                                                       |
    |                              Pointer to next free block                               |
    |                                                                                       |
    +---------------------------------------------------------------------------------------+
    |                                                                                       |
    |                             Pointer to previous free block                            |
    |                                                                                       |
    +---------------------------------------------------------------------------------------+
    |                                                                                       |
										..........
	|                                                                                       |
    +-----------------------------------------------------------------------------+---------+ <- footer
    |                                  block_size                                 |  alloc  |
    |                           (1 LSB implicitly 0)                              |   (0)   |
    |                                    64 bits                                  |  1 bit  |
    +-----------------------------------------------------------------------------+---------+
*/

void* malloc(size_t size);

#endif
