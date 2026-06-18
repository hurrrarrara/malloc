#include "../internal/malloc_internal.h"

static inline void free_impl(void *ptr)
{
	t_heap heap = mm_ptr_to_heap(ptr);
	if (heap.addr == NULL) {
		LOG_INVALID("free", ptr)
		return;
	}

	t_header *blk = mm_pl_to_hdr(ptr);
	if (!mm_is_blk(heap, blk) || mm_has_free_bit(blk)) {
		LOG_INVALID("free", ptr)
		return;
	}

	mm_free_blk(heap, blk);
}

void free(void *ptr)
{
	LOG_FREE_CALL(ptr);
	if (ptr == NULL || !IS_ALIGNED((uintptr_t)ptr)) {
		return;
	}

	mm_lock_mtx();

	mm_init_state();

	free_impl(ptr);

	mm_unlock_mtx();
}
