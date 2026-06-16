#include "../internal/malloc_internal.h"

static inline t_block *alloc_heap(t_heap *heap, size_t size)
{
	t_block *head = mmap(heap->head, size, PROT_READ | PROT_WRITE,
			     MAP_PRIVATE | MAP_ANON, -1, 0);
	if (head == MAP_FAILED)
		return NULL;
	head->size = size;
	set_block_mask(head);
	set_alloc_block(head);
	t_block *footer = get_footer(head);
	footer->next = heap->head;
	set_alloc_block(footer);
	t_free_block *free_block = (t_free_block *)(head + 1);
	free_block->size = size - 2 * sizeof(t_block);
	set_block_mask(free_block);
	set_free_block(free_block);
	get_footer(free_block)->size = free_block->size;
	insert_block(free_block, heap);
	return head;
}

t_block *get_heap_from_ptr(void *ptr)
{
	t_block *start = g_state.tiny.head;
	while (start != NULL) {
		if (ptr >= (void *)((char *)start + sizeof(t_block) * 2) &&
		    ptr <= (void *)((char *)get_footer(start) - MIN_BLOCK_SIZE +
				    sizeof(t_block)))
			return start;
		start = get_footer(start)->next;
	}
	start = g_state.small.head;
	while (start != NULL) {
		if (ptr >= (void *)((char *)start + sizeof(t_block) * 2) &&
		    ptr <= (void *)((char *)get_footer(start) - MIN_BLOCK_SIZE +
				    sizeof(t_block)))
			return start;
		start = get_footer(start)->next;
	}
	start = g_state.big.head;
	while (start != NULL) {
		if (ptr >= (void *)((char *)start + sizeof(t_block) * 2) &&
		    ptr <= (void *)((char *)get_footer(start) - MIN_BLOCK_SIZE +
				    sizeof(t_block)))
			return start;
		start = get_footer(start)->next;
	}

	return NULL;
};

void *expand_heap(t_heap *heap, size_t size)
{
	t_block *head = alloc_heap(heap, heap->size ? heap->size : size);
	if (head == NULL)
		return NULL;
	heap->head = head;
	// write(1, "expand heap: ", 12);
	// write(1, "\n", 1);
	return head;
}

void free_heap(t_heap *heap, t_block *heap_addr, t_free_block *block)
{
	if (block_size(block) != block_size(heap_addr) - sizeof(t_block) * 2 &&
	    heap->size != 0)
		return;
	t_block *prev = NULL;
	t_block *start = heap->head;
	while (start != heap_addr) {
		prev = start;
		start = get_footer(start)->next;
	}
	if (prev == NULL) {
		heap->head = get_footer(heap_addr)->next;
	} else {
		get_footer(prev)->next = get_footer(heap_addr)->next;
	}

	// write(1, "munmap()\n", 9);
	pop_block(block, heap);
	munmap(heap_addr, block_size(heap_addr));
}
