#ifndef MALLOC_INTERNAL_H
#define MALLOC_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>

#define ALIGNMENT _Alignof(max_align_t)
#define IS_ALIGNED(size) ((size) % ALIGNMENT == 0)
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define TINY_PAGE_SIZE (size_t)(sysconf(_SC_PAGESIZE) * 4)
#define TINY_BLOCK_SIZE (size_t)TINY_PAGE_SIZE / 128
#define SMALL_PAGE_SIZE (size_t)(sysconf(_SC_PAGESIZE) * 16)
#define SMALL_BLOCK_SIZE (size_t)TINY_PAGE_SIZE / 128
#define MIN_BLOCK_SIZE (size_t)ALIGN(sizeof(t_free_block) + sizeof(t_block))

#define ALIGN_PAGE(size)                                                       \
	(((size) + (sysconf(_SC_PAGESIZE) - 1)) & ~(ALIGNMENT - 1))

static const size_t block_free_bit = 0x1;

// struct s_block_header {
// 	size_t size;
// } __attribute__((aligned(ALIGNMENT)));
// typedef struct s_block_header t_block_header;
// typedef struct s_block_header t_block_footer;
// typedef struct s_block_header t_block;
//
typedef enum e_class { TINY, SMALL, BIG } t_class;

union s_block {
	size_t size;
	union s_block *next;
} __attribute__((aligned(ALIGNMENT)));
typedef union s_block t_block;

struct s_free_block {
	size_t size;
	struct s_free_block *next;
	struct s_free_block *prev;
} __attribute__((aligned(ALIGNMENT)));
typedef struct s_free_block t_free_block;

typedef struct s_heap {
	t_block *head;
	t_free_block *free_list;
	size_t size;
} t_heap;

typedef struct s_malloc_state {
	t_heap tiny;
	t_heap small;
	t_heap big;
} t_malloc_state;

extern t_malloc_state g_state;

//---------------------------------------------------------------------------//

void *allocate_block(size_t size);
void free_block(t_block *block, t_block *heap_addr);
bool is_valid_block(t_block *block, t_block *heap_header);

void *expand_heap(t_heap *heap, size_t size);
t_block *get_heap_from_ptr(void *ptr);
void *is_valid_ptr(void *ptr);
void free_heap(t_heap *heap, t_block *heap_addr, t_free_block *block);

void insert_block(t_free_block *block, t_heap *heap);
void pop_block(t_free_block *block, t_heap *heap);

//---------------------------------------------------------------------------//

static inline bool is_free_block(void *block)
{
	return ((t_block *)block)->size & block_free_bit;
}

static inline void set_free_block(void *block)
{
	((t_block *)block)->size |= block_free_bit;
}

static inline void set_alloc_block(void *block)
{
	((t_block *)block)->size &= ~block_free_bit;
}

static inline size_t block_size(void *block)
{
	return ((t_block *)block)->size & ~block_free_bit;
}

/*
 *  get the payload from the header
 * */
static inline void *get_payload(void *block)
{
	return (void *)((char *)block + sizeof(t_block));
}

/*
 *  get the footer from header
 *  */
static inline t_block *get_footer(void *block)
{
	return (t_block *)((char *)block + block_size(block) - sizeof(t_block));
}

/*
 *  get the header from footer
 * */
static inline t_block *get_header_from_footer(void *block)
{
	return (t_block *)((char *)block - block_size(block) + sizeof(t_block));
}

/*
 *  get the payload from the header
 * */
static inline t_block *get_header_from_payload(void *payload)
{
	return (t_block *)((char *)payload - sizeof(t_block));
}

/*
 *  get the header of the next block
 * */
static inline t_block *next_block(void *block)
{
	return (t_block *)((char *)block + block_size(block));
}

/*
 *  get the footer of the previous block
 * */
static inline t_block *prev_block(void *block)
{
	return (t_block *)((char *)block - sizeof(t_block));
}

static inline t_block *get_first_block(void *heap)
{
	return (t_block *)((char *)heap + sizeof(t_block));
}

//---------------------------------------------------------------------------//

#define C_RESET "\x1b[0m"
#define C_RED "\x1b[31m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_BLUE "\x1b[34m"
#define C_MAGENTA "\x1b[35m"
#define C_CYAN "\x1b[36m"
#define C_GRAY "\x1b[90m"

static inline void write_str(const char *s)
{
	size_t i = 0;
	while (s[i]) {
		i++;
	};
	write(1, s, i);
}

static inline void write_nbr(size_t n, size_t base)
{
	static char c[] = "0123456789abcdef";

	if (n >= base)
		write_nbr(n / base, base);
	write(1, c + (n % base), 1);
}

static inline void write_ptr(void *ptr)
{
	write_str("0x");
	write_nbr((uintptr_t)ptr, 16);
}

#endif
