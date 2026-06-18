#ifndef MALLOC_INTERNAL_H
#define MALLOC_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>

#define ALIGNMENT _Alignof(max_align_t)
#define IS_ALIGNED(size) ((size) % ALIGNMENT == 0)
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define ALIGN_PAGE(size)                                                       \
	(((size) + (sysconf(_SC_PAGESIZE) - 1)) & ~(ALIGNMENT - 1))

#define TINY_BLOCK_SIZE (size_t)ALIGN(512)
#define TINY_PAGE_SIZE ALIGN_PAGE(TINY_BLOCK_SIZE * 128)
#define SMALL_BLOCK_SIZE (size_t)ALIGN(sysconf(_SC_PAGESIZE) / 2)
#define SMALL_PAGE_SIZE ALIGN_PAGE(SMALL_BLOCK_SIZE * 128)

#define MIN_BLOCK_SIZE (size_t)ALIGN(sizeof(t_free_pl) + 2 * sizeof(t_header))

static const size_t block_free_bit = 0x1;

typedef enum e_class { TINY, SMALL, BIG } t_class;

#ifdef NDEBUG
typedef union s_header {
	size_t size;
	union s_header *next_heap;
} t_header __attribute__((aligned(ALIGNMENT)));
#else

#define CORRUPTION_MASK (uint64_t)0xDEADBEEFCAFEBABE

typedef struct s_header {
	uint64_t mask;
	union {
		size_t size;
		struct s_header *next_heap;
	};
} t_header __attribute__((aligned(ALIGNMENT)));

#endif

typedef struct s_free_pl {
	t_header *next;
	t_header *prev;
} t_free_pl __attribute__((aligned(ALIGNMENT)));

typedef struct s_heap_class {
	t_header *head;
	t_header *free_list;
	size_t size;
	size_t min_blk_size;
} t_heap_class;

typedef struct s_heap {
	t_heap_class *_class;
	t_header *addr;
} t_heap;

typedef struct s_malloc_state {
	t_heap_class tiny;
	t_heap_class small;
	t_heap_class big;
} t_malloc_state;

extern t_malloc_state g_state;

//---------------------------------------------------------------------------//

void mm_lock_mtx();
void mm_unlock_mtx();

t_header *mm_alloc_blk(size_t size);
bool mm_is_blk(t_heap heap, t_header *blk);
void mm_free_blk(t_heap heap, t_header *blk);

t_heap_class *mm_select_heap_class(size_t size);
t_header *mm_grow_heap_class(t_heap_class *heap_class, size_t size);
t_heap mm_ptr_to_heap(void *ptr);
bool mm_heap_empty(t_header *head);
void mm_free_heap(t_heap heap);

void mm_add_free_blk(t_heap_class *heap_class, t_header *blk);
void mm_rm_free_blk(t_heap_class *heap_class, t_header *blk);

t_header *mm_coalesce(t_heap heap, t_header *blk);

void *mm_memset(void *ptr, int value, size_t count);
void *mm_memmove(void *d, const void *s, size_t n);
void *mm_memcpy(void *d, const void *s, size_t n);

//---------------------------------------------------------------------------//

/* clang-format off */

static inline void mm_init_state() {
	g_state.big.size = 0;
	g_state.big.min_blk_size = 0;
	g_state.tiny.size = TINY_PAGE_SIZE;
	g_state.tiny.min_blk_size= MIN_BLOCK_SIZE;
	g_state.small.size = SMALL_PAGE_SIZE;
	g_state.small.min_blk_size = TINY_BLOCK_SIZE;
}

static inline size_t mm_conform_size(size_t size) {
	size = ALIGN(size + 2 * sizeof(t_header));
	size = size < MIN_BLOCK_SIZE ? MIN_BLOCK_SIZE : size;
	// size = size < SMALL_BLOCK_SIZE ? size : ALIGN_PAGE(size + 2 * sizeof(t_header)) - 2 * sizeof(t_header);
	return size;
}

static inline size_t mm_blk_size(t_header *hdr) {
	return (size_t)(hdr->size & ~block_free_bit);
}

static inline size_t mm_pl_size(t_header* hdr) {
	return mm_blk_size(hdr) - 2 * sizeof(t_header);
}

static inline bool mm_has_free_bit(t_header *hdr) {
	return hdr->size & block_free_bit;
}

static inline void mm_set_free_bit(t_header *hdr) {
	hdr->size |= block_free_bit;
}

static inline void mm_rm_free_bit(t_header *hdr) {
	hdr->size &= ~block_free_bit;
}

static inline t_header* mm_next_hdr(t_header *hdr) {
	return (t_header*)((char*)hdr + mm_blk_size(hdr));
}

static inline t_header* mm_prev_ftr(t_header *hdr) {
	return (t_header*)((char*)hdr - sizeof(t_header));
}

static inline t_header* mm_hdr_to_ftr(t_header* hdr) {
	return (t_header*)((char*)hdr + mm_blk_size(hdr) - sizeof(t_header));
}

static inline t_header* mm_ftr_to_hdr(t_header* ftr) {
	return (t_header*)((char*)ftr - mm_blk_size(ftr) + sizeof(t_header));
}

static inline void* mm_hdr_to_pl(t_header* hdr) {
	return (void*)((char*)hdr + sizeof(t_header));
}

static inline t_header* mm_pl_to_hdr(void* pl) {
	return (t_header*)((char*)pl - sizeof(t_header));
}

static inline bool mm_is_corrup(t_header *hdr)
{
#ifndef NDEBUG
	return hdr->mask != CORRUPTION_MASK;
#endif
	(void)hdr;
	return false;
}

static inline void mm_set_hdr_mask(t_header* hdr)
{
#ifndef NDEBUG
	hdr->mask = CORRUPTION_MASK;
#else
	(void)hdr;
#endif
}
/* clang-format on */

//---------------------------------------------------------------------------//

#define C_RESET ""   // "\x1b[0m"
#define C_RED ""     // "\x1b[31m"
#define C_GREEN ""   // "\x1b[32m"
#define C_YELLOW ""  // "\x1b[33m"
#define C_BLUE ""    // "\x1b[34m"
#define C_MAGENTA "" //  "\x1b[35m"
#define C_CYAN ""    //  "\x1b[36m"
#define C_GRAY ""    //  "\x1b[90m"

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

#ifndef NDEBUG
#define LOG(call) write_str(call "\n");
#define LOG_MALLOC_CALL(size)                                                  \
	write_str("malloc call, size: ");                                      \
	write_nbr(size, 10);                                                   \
	write_str("\n");
#define LOG_REALLOC_CALL(ptr, size)                                            \
	write_str("realloc call, ptr: ");                                      \
	write_ptr(ptr);                                                        \
	write_str(" size: ");                                                  \
	write_nbr(size, 10);                                                   \
	write_str("\n");
#define LOG_REALLOCARRAY_CALL(ptr, count, size)                                \
	write_str("reallocarray call, ptr: ");                                 \
	write_ptr(ptr);                                                        \
	write_str(" count: ");                                                 \
	write_nbr(count, 10);                                                  \
	write_str(" size: ");                                                  \
	write_nbr(size, 10);                                                   \
	write_str("\n");
#define LOG_CALLOC_CALL(count, size)                                           \
	write_str("calloc call, count: ");                                     \
	write_nbr(count, 10);                                                  \
	write_str(" size: ");                                                  \
	write_nbr(size, 10);                                                   \
	write_str("\n");
#define LOG_FREE_CALL(ptr)                                                     \
	write_str("free call, ptr: ");                                         \
	write_ptr(ptr);                                                        \
	write_str("\n");

#define LOG_MALLOC_RET(ptr)                                                    \
	write_str("malloc -> ");                                               \
	write_ptr(ptr);                                                        \
	write_str("\n");
#define LOG_REALLOC_RET(ptr)                                                   \
	write_str("realloc -> ");                                              \
	write_ptr(ptr);                                                        \
	write_str("\n");
#define LOG_REALLOCARRAY_RET(ptr)                                              \
	write_str("reallocarray -> ");                                         \
	write_ptr(ptr);                                                        \
	write_str("\n");
#define LOG_CALLOC_RET(ptr)                                                    \
	write_str("calloc -> ");                                               \
	write_ptr(ptr);                                                        \
	write_str("\n");

#define LOG_ALLOC_FAIL(call, size)                                             \
	write_str(call " alloc fail : ");                                      \
	write_nbr(size, 10);                                                   \
	write_str("\n");

#define LOG_FREE_FAIL(call, ptr)                                               \
	write_str(call " free fail : ");                                       \
	write_ptr(ptr);                                                        \
	write_str("\n");

#define LOG_INVALID(call, ptr)                                                 \
	write_str(call " invalid : ");                                         \
	write_ptr(ptr);                                                        \
	write_str("\n");
#else
#define LOG(call)
#define LOG_MALLOC_CALL(size)
#define LOG_REALLOC_CALL(ptr, size)
#define LOG_REALLOCARRAY_CALL(ptr, count, size)
#define LOG_CALLOC_CALL(count, size)
#define LOG_FREE_CALL(ptr)

#define LOG_MALLOC_RET(ptr)
#define LOG_REALLOC_RET(ptr)
#define LOG_REALLOCARRAY_RET(ptr)
#define LOG_CALLOC_RET(ptr)

#define LOG_ALLOC_FAIL(call, size)
#define LOG_FREE_FAIL(call, size)
#define LOG_INVALID(call, ptr)
#endif
#endif
