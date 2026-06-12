#include "malloc.h"
#include <stddef.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>

#include <assert.h> 


#define ALIGNMENT _Alignof(max_align_t)
#define IS_ALIGNED(size)  ((size) % ALIGNMENT == 0)
#define ALIGN(size)  (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define PAGE_SIZE sysconf(_SC_PAGESIZE) * 4
#define MAX_BLOCK_SIZE (size_t)PAGE_SIZE / 128
#define MIN_BLOCK_SIZE (size_t)ALIGN(sizeof(t_free_block) + sizeof(t_block_footer))

static const size_t block_free_bit = 0x1;

struct s_block_header {
	size_t size;
} __attribute__ ((aligned (ALIGNMENT)));
typedef struct s_block_header t_block_header;
typedef struct s_block_header t_block_footer;
typedef struct s_block_header t_block;

struct s_free_block {
	size_t size;
	struct s_free_block *next;
	struct s_free_block *prev;
} __attribute__ ((aligned (ALIGNMENT)));
typedef struct s_free_block t_free_block;

struct s_heap {
	t_block_header	*head;
	t_free_block 	*free_list;
	size_t			size;
};
typedef struct s_heap t_heap;


// #define is_free_block(block) (block->size & block_free_bit)
// #define set_free_block(block) block->size |= block_free_bit
// #define set_alloc_block(block) block->size &= ~block_free_bit
// #define is_not_initialised(x) (x.head == NULL && x.free_list == NULL && x.size == 0)
// #define get_footer(head, size) ((t_block_footer*)((char *)head + size - sizeof(t_block_header)))
// #define get_next_block(head, size) ((void*)((char*)head + size))
// #define block_size(block) (block->size & ~block_free_bit)
// #define prev_block_footer(block) ((t_block_footer*)((char *)block - sizeof(t_block_footer)))
// #define next_block_header(block) ((t_block_header*)((char *)block + block_size(block)))
// #define header_from_footer(block) ((t_block_header*)((char *)block + sizeof(t_block_footer) - block_size(block)))
// #define get_paylod(block) ((void*)((char*)block + sizeof(t_block_header)))
// #define get_header_from_payload(ptr) ((void*)((char*)ptr - sizeof(t_block_header)))

static inline bool is_free_block(void* block) {
	return ((t_block*)block)->size & block_free_bit;
}

static inline void set_free_block(void* block) {
	((t_block*)block)->size |=  block_free_bit;
}

static inline void set_alloc_block(void* block) {
	((t_block*)block)->size &= ~block_free_bit;
}

static inline size_t block_size(void* block) {
	return ((t_block*)block)->size & ~block_free_bit;
}

/*
 *  get the payload from the header
 * */
static inline void* get_payload(void* block) {
	return (void*)((char*)block + sizeof(t_block));
}

/*
 *  get the footer from header
 *  */
static inline t_block* get_footer(void* block) {
	return (t_block*)((char*)block + block_size(block) - sizeof(t_block));
}

/*
 *  get the header from footer 
 * */
static inline t_block* get_header_from_footer(void* block) {
	return (t_block*)((char*)block - block_size(block) + sizeof(t_block));
}

/*
 *  get the payload from the header
 * */
static inline t_block* get_header_from_payload(void* payload) {
	return (t_block*)((char*)payload - sizeof(t_block));
}

/*
 *  get the header of the next block
 * */
static inline t_block* next_block(void* block) {
	return (t_block*)((char*)block + block_size(block));
}

/*
 *  get the footer of the previous block
 * */
static inline t_block* prev_block(void* block) {
	return (t_block*)((char*)block - sizeof(t_block));
}


 


_Static_assert(sizeof(t_block_header) % ALIGNMENT == 0, "block_header_t must be aligned");
_Static_assert(sizeof(t_free_block) % ALIGNMENT == 0, "free_block_t must be aligned");
_Static_assert((ALIGNMENT & (ALIGNMENT - 1)) == 0, "ALIGNMENT must be a power of 2");

static t_heap g_heap = {.head = NULL, .free_list = NULL, .size = 0};


int malloc_init() {
	g_heap.size = PAGE_SIZE;
	g_heap.head = mmap(NULL, g_heap.size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if (g_heap.head == MAP_FAILED)
		return 1;
	set_alloc_block(g_heap.head);
	set_alloc_block((t_block*)((char*)g_heap.head + g_heap.size - sizeof(t_block)));
	t_free_block* free_block = (t_free_block*)(g_heap.head + 1);
	free_block->size = g_heap.size - 2 * sizeof(t_block);
	free_block->prev = NULL;
	free_block->next = NULL;
	set_free_block(free_block);
	get_footer(free_block)->size = free_block->size;
	g_heap.free_list = free_block;
	return 0;
}

t_free_block* find_free_block(size_t size) {
	t_free_block* start = g_heap.free_list;
	while (start) {
		if (size <= block_size(start))
			return start;
		start = start->next;
	}
	return NULL;
};


static inline void* split_block(t_free_block* block, size_t size) {
	size_t split_size = block_size(block) - size;
	block->size = split_size;

	t_block* alloc_block = next_block(block);
	alloc_block->size = size;
	set_alloc_block(alloc_block);
	get_footer(alloc_block)->size = alloc_block->size;

	set_free_block(block);
	get_footer(block)->size = block->size;

	return alloc_block;
}

t_block_header* alloc_block(t_free_block* block, size_t size) {
	if (block_size(block) - size >= MIN_BLOCK_SIZE)
		return split_block(block, size);
	set_alloc_block(block);
	get_footer(block)->size = block->size;
	if (block->next)
		block->next->prev = block->prev;
	if (block->prev)
		block->prev->next = block->next;
	else
		g_heap.free_list = block->next;
	return (t_block_header*)block;
}

void* malloc(size_t size) {
	if (size == 0 || size > MAX_BLOCK_SIZE)
		return NULL;
	size = ALIGN(size + 2 * sizeof(t_block_header));
	size = size < MIN_BLOCK_SIZE ? MIN_BLOCK_SIZE : size;

	if (g_heap.head == NULL)
		if (malloc_init())
			return NULL;
	t_free_block* free_block = find_free_block(size);
	if (free_block == NULL)
		return NULL;
	return get_payload(alloc_block(free_block, size));
}

static inline void pop_block(t_free_block *block) {
	if (block->next != NULL)
		block->next->prev = block->prev;
	if (block->prev != NULL)
		block->prev->next = block->next;
}

static inline void insert_block(t_free_block* block) {
	block->next = NULL;
	if (block != g_heap.free_list)
		block->next = g_heap.free_list;
	block->prev = NULL;
	g_heap.free_list = block;
}

static inline void* coalesce(t_free_block *block) {
	t_block_header* prev = prev_block(block);
	t_block_header* next = next_block(block);

	char prev_free = is_free_block(prev);
	char next_free = is_free_block(next);

	if (prev_free && !next_free) {
		prev = get_header_from_footer(prev);
		pop_block((t_free_block*)prev);
		prev->size = block_size(prev) + block_size(block);
		set_free_block(prev);
		block = (t_free_block*)prev;
	}
	if (!prev_free && next_free) {
		pop_block((t_free_block*)next);
		block->size = block_size(block) + block_size(next);
		set_free_block(block);
	}
	if (prev_free && next_free) {
		prev = get_header_from_footer(prev);
		pop_block((t_free_block*)prev);
		pop_block((t_free_block*)next);
		prev->size = block_size(prev) + block_size(block) + block_size(next);
		set_free_block(prev);
		block = (t_free_block*)prev;
	}
	assert(block->size <= g_heap.size - 2 * sizeof(t_block));

	insert_block(block);
	set_free_block(block);
	get_footer(block)->size = block->size;
	return block;
}

static inline int is_valid_block(t_block_header* block) {
	return is_free_block(block) == 0;
}

static inline void free_heap() {
	write(1, "munmap()\0", 9);
	munmap(g_heap.head, g_heap.size);
	g_heap.head = NULL;
	g_heap.free_list = NULL;
	g_heap.size = 0;
}

void free(void* ptr) {
	write(1, "free()\0", 7);
	//check if ptr is valid
	if (ptr == NULL || !is_valid_block(ptr))
		return;
	t_free_block* block = (t_free_block*)get_header_from_payload(ptr);
	block = coalesce(block);
	if (block_size(block) == g_heap.size - 2 * sizeof(t_block_header))
		free_heap();

}
