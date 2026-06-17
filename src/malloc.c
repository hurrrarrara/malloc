#include "../internal/malloc_internal.h"

t_malloc_state g_state = {0};

void *malloc_impl(size_t size)
{
	size = mm_conform_size(size);

	t_header *blk = mm_alloc_blk(size);

	return blk == NULL ? NULL : mm_hdr_to_pl(blk);
}

void *malloc(size_t size)
{
	LOG_MALLOC_CALL(size);
	if (size == 0)
		return NULL;

	mm_lock_mtx();
	g_state.big.size = 0;
	g_state.tiny.size = TINY_PAGE_SIZE;
	g_state.small.size = SMALL_PAGE_SIZE;

	void *ret = malloc_impl(size);

	if (ret == NULL) {
		LOG_ALLOC_FAIL("malloc", size);
	}

	mm_unlock_mtx();

	LOG_MALLOC_RET(ret);

	return ret;
}
