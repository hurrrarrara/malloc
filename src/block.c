#include "internal/malloc_internal.h"

static inline void *split_block(t_free_block *block, size_t size)
{
	size_t split_size = block_size(block) - size;
	block->size = split_size;

	t_block *alloc_block = next_block(block);
	alloc_block->size = size;
	set_alloc_block(alloc_block);
	get_footer(alloc_block)->size = alloc_block->size;

	set_free_block(block);
	get_footer(block)->size = block->size;

	return alloc_block;
}

static inline t_free_block *find_free_block(size_t size, t_heap *heap)
{
	t_free_block *start = heap->free_list;
	while (start) {
		if (size <= block_size(start))
			return start;
		start = start->next;
	}
	if (expand_heap(heap, size) == NULL)
		return NULL;
	return find_free_block(size, heap);
}

static inline void *coalesce(t_free_block *block, t_heap *heap)
{
	t_block *prev = prev_block(block);
	t_block *next = next_block(block);

	bool prev_free = is_free_block(prev);
	bool next_free = is_free_block(next);

	if (prev_free && !next_free) {
		prev = get_header_from_footer(prev);
		pop_block((t_free_block *)prev, heap);
		prev->size = block_size(prev) + block_size(block);
		set_free_block(prev);
		block = (t_free_block *)prev;
	}
	if (!prev_free && next_free) {
		pop_block((t_free_block *)next, heap);
		block->size = block_size(block) + block_size(next);
		set_free_block(block);
	}
	if (prev_free && next_free) {
		prev = get_header_from_footer(prev);
		pop_block((t_free_block *)prev, heap);
		pop_block((t_free_block *)next, heap);
		prev->size =
		    block_size(prev) + block_size(block) + block_size(next);
		set_free_block(prev);
		block = (t_free_block *)prev;
	}
	// assert(block_size(block) <= g_heap.size - 2 * sizeof(t_block));

	insert_block(block, heap);
	set_free_block(block);
	get_footer(block)->size = block->size;
	return block;
}

void *allocate_block(size_t size)
{
	t_heap *heap =
	    size <= TINY_BLOCK_SIZE
		? &g_state.tiny
		: (size <= SMALL_BLOCK_SIZE ? &g_state.small : &g_state.big);
	t_free_block *block = find_free_block(size, heap);
	if (block == NULL)
		return NULL;
	if ((size <= TINY_BLOCK_SIZE &&
	     block_size(block) - size > MIN_BLOCK_SIZE) ||
	    (size <= SMALL_BLOCK_SIZE &&
	     block_size(block) - size > TINY_BLOCK_SIZE)) {
		return split_block(block, size);
	}
	pop_block(block, heap);
	set_alloc_block(block);
	return block;
}

bool is_valid_block(t_block *block, t_block *heap_addr)
{
	if (block_size(heap_addr) == TINY_PAGE_SIZE &&
		block_size(block) > TINY_BLOCK_SIZE ||
	    block_size(heap_addr) == SMALL_PAGE_SIZE &&
		block_size(block) > SMALL_BLOCK_SIZE)
		return false;
	t_block *start = (t_block *)((char *)heap_addr + sizeof(t_block));
	t_block *end = get_footer(heap_addr);
	while (start != end) {
		if (start == (t_block *)block)
			return true;
		start = next_block(start);
	}
	return false;
}

void free_block(t_block *block, t_block *heap_addr)
{
	t_heap *heap =
	    (block_size(heap_addr) == TINY_PAGE_SIZE
		 ? &g_state.tiny
		 : (block_size(heap_addr) == SMALL_PAGE_SIZE ? &g_state.small
							     : &g_state.big));
	block = coalesce((t_free_block *)block, heap);
	free_heap(heap, heap_addr, (t_free_block *)block);
}
