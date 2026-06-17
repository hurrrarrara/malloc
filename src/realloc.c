#include "../includes/malloc.h"
#include "../internal/malloc_internal.h"
#include <stdint.h>

static inline void *realloc_impl(void *ptr, size_t size)
{
	t_heap heap = mm_ptr_to_heap(ptr);
	if (heap.addr == NULL) {
		LOG_INVALID("realloc", ptr);
		return NULL;
	}
	t_header *blk = mm_pl_to_hdr(ptr);
	if (!mm_is_blk(heap, blk) || mm_has_free_bit(blk)) {
		LOG_INVALID("realloc", ptr);
		return NULL;
	}

	size = mm_conform_size(size);
	size_t old_size = mm_blk_size(blk);
	if (old_size >= size) {
		// LOG_INVALID("realloc did nothing", ptr)
		return ptr;
	}

	t_header *new_blk = mm_alloc_blk(size);
	if (new_blk == NULL) {
		LOG_ALLOC_FAIL("realloc", size);
		return NULL;
	}

	void *new_ptr = mm_hdr_to_pl(new_blk);
	size = old_size > size ? mm_pl_size(new_blk) : mm_pl_size(blk);
	for (size_t i = 0; i < size; i++) {
		((char *)new_ptr)[i] = ((char *)ptr)[i];
	}

	mm_free_blk(heap, blk);
	return new_ptr;
}

void *realloc(void *ptr, size_t size)
{

	if (ptr == NULL)
		return malloc(size);
	if (size == 0) {
		LOG("realloc size 0");
		free(ptr);
		return NULL;
	}
	if (!IS_ALIGNED((size_t)ptr)) {
		LOG_INVALID("realloc aligned", ptr);
		return NULL;
	}

	mm_lock_mtx();
	g_state.big.size = 0;
	g_state.tiny.size = TINY_PAGE_SIZE;
	g_state.small.size = SMALL_PAGE_SIZE;

	void *new_ptr = realloc_impl(ptr, size);

	mm_unlock_mtx();
	return new_ptr;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
	if (size != 0 && nmemb > SIZE_MAX / size) {
		// errno = ENOMEM;
		return NULL;
	}
	LOG("reallocarray");
	return realloc(ptr, nmemb * size);
}
