#include "../includes/malloc.h"
#include "../internal/malloc_internal.h"
#include <errno.h>

static void *realloc_smaller_size(t_heap heap, t_header *blk, size_t size,
				  size_t old_size)
{
	blk->size = size;
	*mm_hdr_to_ftr(blk) = *blk;
	t_header *free_blk = mm_next_hdr(blk);
	free_blk->size = old_size - size;
	mm_set_free_bit(free_blk);
	*mm_hdr_to_ftr(free_blk) = *free_blk;
	mm_add_free_blk(heap._class, free_blk);
	return mm_hdr_to_pl(blk);
}

static void *realloc_impl(void *ptr, size_t size)
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
		if (old_size - size >= heap._class->min_blk_size &&
		    heap._class->min_blk_size > 0)
			return realloc_smaller_size(heap, blk, size, old_size);
		return ptr;
	}

	t_header *new_blk = mm_alloc_blk(size);
	if (new_blk == NULL) {
		LOG_ALLOC_FAIL("realloc", size);
		return NULL;
	}

	void *new_ptr = mm_hdr_to_pl(new_blk);
	size = old_size > size ? mm_pl_size(new_blk) : mm_pl_size(blk);
	mm_memcpy(new_ptr, ptr, size);

	mm_free_blk(heap, blk);
	return new_ptr;
}

void *realloc(void *ptr, size_t size)
{

	LOG_REALLOC_CALL(ptr, size)
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

	mm_init_state();

	void *new_ptr = realloc_impl(ptr, size);

	mm_unlock_mtx();

	LOG_REALLOC_RET(new_ptr);
	return new_ptr;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
	LOG_REALLOCARRAY_CALL(ptr, nmemb, size)
	if (size != 0 && nmemb > SIZE_MAX / size) {
		errno = ENOMEM;
		return NULL;
	}
	void *new_ptr = realloc(ptr, nmemb * size);
	LOG_REALLOCARRAY_RET(new_ptr);
	return new_ptr;
}
