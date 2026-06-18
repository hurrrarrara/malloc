#include "../internal/malloc_internal.h"

void *malloc_impl(size_t size)
{
	size = mm_conform_size(size);

	t_header *blk = mm_alloc_blk(size);

	return blk == NULL ? NULL : mm_hdr_to_pl(blk);
}

void *malloc(size_t size)
{
	LOG_MALLOC_CALL(size);

	mm_lock_mtx();

	mm_init_state();

	void *ret = malloc_impl(size);

	if (ret == NULL) {
		LOG_ALLOC_FAIL("malloc", size);
	}

	mm_unlock_mtx();

	LOG_MALLOC_RET(ret);

	return ret;
}
