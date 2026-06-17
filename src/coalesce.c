#include "../internal/malloc_internal.h"

t_header *mm_coalesce(t_heap heap, t_header *blk)
{
	t_header *prev = mm_prev_ftr(blk);
	t_header *next = mm_next_hdr(blk);

	if (mm_has_free_bit(prev) && !mm_has_free_bit(next)) {
		prev = mm_ftr_to_hdr(prev);
		mm_rm_free_blk(heap._class, prev);
		prev->size = mm_blk_size(prev) + mm_blk_size(blk);
		blk = prev;
	} else if (!mm_has_free_bit(prev) && mm_has_free_bit(next)) {
		mm_rm_free_blk(heap._class, next);
		blk->size = mm_blk_size(blk) + mm_blk_size(next);
	} else if (mm_has_free_bit(prev) && mm_has_free_bit(next)) {
		prev = mm_ftr_to_hdr(prev);
		mm_rm_free_blk(heap._class, prev);
		mm_rm_free_blk(heap._class, next);
		prev->size =
		    mm_blk_size(prev) + mm_blk_size(blk) + mm_blk_size(next);
		blk = prev;
	}

	mm_set_free_bit(blk);
	*mm_hdr_to_ftr(blk) = *blk;
	mm_add_free_blk(heap._class, blk);
	return blk;
}
