#include "malloc.h"
#include "../internal/malloc_internal.h"
#include <asm-generic/errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdint.h>

t_malloc_state g_state = {0};
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// __attribute__((constructor)) static inline void malloc_init(void)
// {
// 	g_state.big.size = 0;
// 	g_state.tiny.size = TINY_PAGE_SIZE;
// 	g_state.small.size = SMALL_PAGE_SIZE;
// }

void *malloc(size_t size)
{
	// write_str("MALLOC\n");

	if (size == 0)
		return NULL;
	size = precompute_size(size);

	// pthread_mutex_lock(&g_mutex);
	pthread_mutex_lock(&g_mutex);
	g_state.big.size = 0;
	g_state.tiny.size = TINY_PAGE_SIZE;
	g_state.small.size = SMALL_PAGE_SIZE;

	void *ptr = allocate_block(size);

	if (ptr == NULL) {
		pthread_mutex_unlock(&g_mutex);
		return NULL;
	}
	pthread_mutex_unlock(&g_mutex);

	return get_payload(ptr);
}

void free(void *ptr)
{

	if (ptr == NULL || !IS_ALIGNED((size_t)ptr))
		return;
	// write_str("FREE: ");
	// write_ptr(ptr);
	// write_str("\n");

	// pthread_mutex_lock(&g_mutex);
	pthread_mutex_lock(&g_mutex);
	g_state.big.size = 0;
	g_state.tiny.size = TINY_PAGE_SIZE;
	g_state.small.size = SMALL_PAGE_SIZE;

	t_block *heap_addr = get_heap_from_ptr(ptr);
	// write_str("free 1if\n\0");
	if (heap_addr == NULL) {
		pthread_mutex_unlock(&g_mutex);
		return;
	}
	t_block *block = get_header_from_payload(ptr);
	// write_str("free 2if\n\0");
	if (!is_valid_block((t_block *)block, heap_addr) ||
	    is_free_block(block)) {
		pthread_mutex_unlock(&g_mutex);
		return;
	}

	free_allocated_block(block, heap_addr);
	// write_str("free end\n\0");
	pthread_mutex_unlock(&g_mutex);
	return;
}

static inline void *realloc_impl(void *ptr, size_t size)
{
	t_block *heap_addr = get_heap_from_ptr(ptr);
	if (heap_addr == NULL)
		return NULL;
	t_block *block = get_header_from_payload(ptr);

	if (size == 0) {
		free_allocated_block(block, heap_addr);
		return NULL;
	}

	size = precompute_size(size);

	if (!is_valid_block(block, heap_addr) || is_free_block(block))
		return NULL;

	size_t old_size = block_size(block);
	if (old_size >= size)
		return ptr;

	t_block *new_block = allocate_block(size);
	if (new_block == NULL) {
		return NULL;
	}
	void *new_ptr = get_payload(new_block);
	size = old_size > size ? payload_size(new_block) : payload_size(block);
	for (size_t i = 0; i < size; i++) {
		((char *)new_ptr)[i] = ((char *)ptr)[i];
	}
	free_allocated_block(block, heap_addr);
	return new_ptr;
}

void *realloc(void *ptr, size_t size)
{

	// write_str("REALLOC\n");
	if (ptr == NULL)
		return malloc(size);
	if (!IS_ALIGNED((size_t)ptr))
		return NULL;

	// pthread_mutex_lock(&g_mutex);
	pthread_mutex_lock(&g_mutex);
	g_state.big.size = 0;
	g_state.tiny.size = TINY_PAGE_SIZE;
	g_state.small.size = SMALL_PAGE_SIZE;

	void *new_ptr = realloc_impl(ptr, size);

	pthread_mutex_unlock(&g_mutex);
	return new_ptr;
}

void *calloc(size_t count, size_t size)
{
	// write_str("CALLOC\n");
	void *ptr = malloc(count * size);
	if (ptr == NULL)
		return NULL;
	for (size_t i = 0; i < count * size; i++)
		((char *)ptr)[i] = 0;
	return ptr;
}
