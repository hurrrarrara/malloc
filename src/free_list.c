#include "../internal/malloc_internal.h"

void mm_add_free_blk(t_heap_class *heap_class, t_header *hdr)
{
	t_free_pl *pl = mm_hdr_to_pl(hdr);
	pl->next = NULL;
	if (hdr != heap_class->free_list)
		pl->next = heap_class->free_list;
	if (heap_class->free_list != NULL)
		((t_free_pl *)mm_hdr_to_pl(heap_class->free_list))->prev = hdr;
	pl->prev = NULL;
	heap_class->free_list = hdr;
}

void mm_rm_free_blk(t_heap_class *heap_class, t_header *hdr)
{
	t_free_pl *pl = mm_hdr_to_pl(hdr);
	if (heap_class->free_list == hdr)
		heap_class->free_list = pl->prev != NULL ? pl->prev : pl->next;
	if (pl->next != NULL)
		((t_free_pl *)mm_hdr_to_pl(pl->next))->prev = pl->prev;
	if (pl->prev != NULL)
		((t_free_pl *)mm_hdr_to_pl(pl->prev))->next = pl->next;
}
