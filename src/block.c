#include "../internal/malloc_internal.h"

static inline t_header *get_free_blk(t_heap *heap, size_t size)
{
	t_header *start = heap->free_list;
	while (start) {
		if (size <= mm_blk_size(start))
			return start;
		start = ((t_free_pl *)mm_hdr_to_pl(start))->next;
	}
	if (mm_grow_heap(heap, size + 2 * sizeof(t_header)) == NULL)
		return NULL;
	return get_free_blk(heap, size);
}

static inline void *split_block(t_header *blk, size_t size)
{
	size_t split_size = mm_blk_size(blk) - size;
	blk->size = split_size;
	mm_set_free_bit(blk);
	*mm_hdr_to_ftr(blk) = *blk;

	t_header *alloc_blk = mm_next_hdr(blk);
	alloc_blk->size = size;
	mm_set_hdr_mask(alloc_blk);
	*mm_hdr_to_ftr(alloc_blk) = *alloc_blk;

	return alloc_blk;
}

t_header *mm_alloc_blk(size_t size)
{
	t_heap heap = mm_select_heap(size);
	t_header *blk = get_free_blk(&heap, size);
	if (blk == NULL || mm_is_corrup(blk))
		return NULL;
	if ((size <= TINY_BLOCK_SIZE &&
	     mm_blk_size(blk) - size > MIN_BLOCK_SIZE) ||
	    (size <= SMALL_BLOCK_SIZE &&
	     mm_blk_size(blk) - size > TINY_BLOCK_SIZE)) {
		return split_block(blk, size);
	}
	mm_rm_free_blk(&heap, blk);
	mm_rm_free_bit(blk);
	*mm_hdr_to_ftr(blk) = *blk;

	return blk;
}
