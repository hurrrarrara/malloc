#include "../internal/malloc_internal.h"

t_heap mm_select_heap(size_t size)
{
	return size <= TINY_BLOCK_SIZE
		   ? g_state.tiny
		   : (size <= SMALL_BLOCK_SIZE ? g_state.small : g_state.big);
}

// void *expand_heap(t_heap *heap, size_t size)
// {
// 	t_block *head = alloc_heap(heap, heap->size ? heap->size : size);
// 	if (head == NULL)
// 		return NULL;
// 	heap->head = head;
// 	// write(1, "expand heap: ", 12);
// 	// write(1, "\n", 1);
// 	return head;
// }

// static inline t_block *alloc_heap(t_heap *heap, size_t size)
// {
// 	t_block *head = mmap(heap->head, size, PROT_READ | PROT_WRITE,
// 			     MAP_PRIVATE | MAP_ANON, -1, 0);
// 	if (head == MAP_FAILED)
// 		return NULL;
// 	head->size = size;
// 	set_block_mask(head);
// 	set_alloc_block(head);
// 	t_block *footer = get_footer(head);
// 	footer->next = heap->head;
// 	set_alloc_block(footer);
// 	t_free_block *free_block = (t_free_block *)(head + 1);
// 	free_block->size = size - 2 * sizeof(t_block);
// 	set_block_mask(free_block);
// 	set_free_block(free_block);
// 	get_footer(free_block)->size = free_block->size;
// 	insert_block(free_block, heap);
// 	return head;
// }

t_header *mm_grow_heap(t_heap *heap, size_t size)
{
	t_header *head =
	    mmap(heap->head, heap->size ? heap->size : size,
		 PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if (head == MAP_FAILED)
		return NULL;
	head->size = size;
	mm_set_hdr_mask(head);
	mm_hdr_to_ftr(head)->next_heap = heap->head;
	heap->head = head;

	t_header *blk = mm_hdr_to_pl(head);
	blk->size = mm_pl_size(head);
	mm_set_hdr_mask(blk);
	mm_set_free_bit(blk);
	*mm_hdr_to_ftr(blk) = *blk;
	mm_add_free_blk(heap, blk);

	return head;
}
