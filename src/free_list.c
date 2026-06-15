#include "internal/malloc_internal.h"

void insert_block(t_free_block *block, t_heap *heap)
{
	block->next = NULL;
	if (block != heap->free_list)
		block->next = heap->free_list;
	if (heap->free_list != NULL)
		(heap->free_list)->prev = block;
	block->prev = NULL;
	heap->free_list = block;
}
void pop_block(t_free_block *block, t_heap *heap)
{
	if (heap->free_list == block)
		heap->free_list =
		    block->prev != NULL ? block->prev : block->next;
	if (block->next != NULL)
		block->next->prev = block->prev;
	if (block->prev != NULL)
		block->prev->next = block->next;
}
