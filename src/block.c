#include "../internal/malloc_internal.h"

static inline t_header *get_free_blk(t_heap_class *heap_class, size_t size)
{
	t_header *start = heap_class->free_list;
	while (start) {
		if (size <= mm_blk_size(start))
			return start;
		start = ((t_free_pl *)mm_hdr_to_pl(start))->next;
	}
	if (mm_grow_heap_class(heap_class, size + 2 * sizeof(t_header)) == NULL)
		return NULL;
	return get_free_blk(heap_class, size);
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
	t_heap_class *heap_class = mm_select_heap_class(size);
	t_header *blk = get_free_blk(heap_class, size);
	if (blk == NULL || mm_is_corrup(blk))
		return NULL;
	if ((size <= TINY_BLOCK_SIZE &&
	     mm_blk_size(blk) - size > MIN_BLOCK_SIZE) ||
	    (size <= SMALL_BLOCK_SIZE &&
	     mm_blk_size(blk) - size > TINY_BLOCK_SIZE)) {
		return split_block(blk, size);
	}
	mm_rm_free_blk(heap_class, blk);
	mm_rm_free_bit(blk);
	*mm_hdr_to_ftr(blk) = *blk;

	return blk;
}

static inline bool mm_blk_in_heap(t_header *head, t_header *blk)
{
#ifndef NDEBUG
	t_header *start = mm_hdr_to_pl(head);
	t_header *end = mm_hdr_to_ftr(head);
	while (start != end) {
		if (start == blk)
			return true;
		start = mm_next_hdr(start);
	}
	return false;
#endif
	(void)head;
	(void)blk;
	return true;
}

bool mm_is_blk(t_heap heap, t_header *blk)
{
	if (mm_is_corrup(blk) || mm_blk_size(blk) > mm_pl_size(heap.addr))
		return false;
	return mm_blk_in_heap(heap.addr, blk);
}

void mm_free_blk(t_heap heap, t_header *blk)
{
	blk = mm_coalesce(heap, blk);
	if (mm_heap_empty(heap.addr)) {
		mm_rm_free_blk(heap._class, blk);
		mm_free_heap(heap);
	}
}
