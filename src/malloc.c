#include "malloc.h"
#include "../internal/malloc_internal.h"

t_malloc_state g_state = {0};

void *malloc(size_t size)
{
	g_state.big.size = 0;
	g_state.tiny.size = TINY_PAGE_SIZE;
	g_state.small.size = SMALL_PAGE_SIZE;
	// thread synchro
	// if (size == 0 || size > MAX_BLOCK_SIZE)
	// 	return NULL;
	size = ALIGN(size + 2 * sizeof(t_block));
	size = size < MIN_BLOCK_SIZE ? MIN_BLOCK_SIZE : size;

	void *ptr = allocate_block(size);

	return get_payload(ptr);
}

void free(void *ptr)
{
	if (ptr == NULL || !IS_ALIGNED((size_t)ptr))
		return;

	t_block *heap_addr = get_heap_from_ptr(ptr);
	if (heap_addr == NULL) {
		write(1, "heap find failed: ", 23);
		return;
	}
	t_block *block = get_header_from_payload(ptr);
	if (!is_valid_block((t_block *)block, heap_addr) ||
	    is_free_block(block))
		return;

	free_block(block, heap_addr);
}

void *realloc(void *ptr, size_t size)
{
	if (!IS_ALIGNED((size_t)ptr))
		return NULL;
	if (ptr == NULL)
		return malloc(size);

	t_block *heap_addr = get_heap_from_ptr(ptr);
	if (heap_addr == NULL)
		return NULL;
	if (size == 0) {
		free_block(ptr, heap_addr);
		return NULL;
	}

	size = ALIGN(size + 2 * sizeof(t_block));
	size = size < MIN_BLOCK_SIZE ? MIN_BLOCK_SIZE : size;
	t_block *block = get_header_from_payload(ptr);

	if (!is_valid_block(block, heap_addr) || is_free_block(block))
		return NULL;

	size_t old_size = block_size(block);
	if (old_size == size)
		return ptr;

	void *new_ptr = allocate_block(size);
	size = old_size > size ? size : old_size;
	for (size_t i = 0; i < size; i++) {
		((char *)new_ptr)[i] = ((char *)ptr)[i];
	}
	free_block(block, heap_addr);
	return new_ptr;
}
