#include "../internal/malloc_internal.h"

void mm_add_free_blk(t_heap *heap, t_header *hdr)
{
	t_free_pl *pl = mm_hdr_to_pl(hdr);
	pl->next = NULL;
	if (hdr != heap->free_list)
		pl->next = heap->free_list;
	if (heap->free_list != NULL)
		((t_free_pl *)mm_hdr_to_pl(heap->free_list))->prev = hdr;
	pl->prev = NULL;
	heap->free_list = hdr;
}

void mm_rm_free_blk(t_heap *heap, t_header *hdr)
{
	t_free_pl *pl = mm_hdr_to_pl(hdr);
	if (heap->free_list == hdr)
		heap->free_list = pl->prev != NULL ? pl->prev : pl->next;
	if (pl->next != NULL)
		((t_free_pl *)mm_hdr_to_pl(pl->next))->prev = pl->prev;
	if (pl->prev != NULL)
		((t_free_pl *)mm_hdr_to_pl(pl->prev))->next = pl->next;
}
