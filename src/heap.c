#include "../internal/malloc_internal.h"
#include <sys/mman.h>

t_heap_class *mm_select_heap_class(size_t size)
{
	return size <= TINY_BLOCK_SIZE
		   ? &g_state.tiny
		   : (size <= SMALL_BLOCK_SIZE ? &g_state.small : &g_state.big);
}

static inline bool ptr_in_heap(t_header *head, void *ptr)
{
	return ptr >= mm_hdr_to_pl((t_header *)mm_hdr_to_pl(head)) &&
	       ptr <= (void *)((char *)mm_hdr_to_ftr(head) - MIN_BLOCK_SIZE +
			       sizeof(t_header));
}

t_heap mm_ptr_to_heap(void *ptr)
{
	t_heap heap;
	heap._class = &g_state.tiny;
	t_header *start = heap._class->head;
	while (start != NULL) {
		if (ptr_in_heap(start, ptr)) {
			heap.addr = start;
			return heap;
		}
		start = mm_hdr_to_ftr(start)->next_heap;
	}
	heap._class = &g_state.small;
	start = heap._class->head;
	while (start != NULL) {
		if (ptr_in_heap(start, ptr)) {
			heap.addr = start;
			return heap;
		}
		start = mm_hdr_to_ftr(start)->next_heap;
	}
	heap._class = &g_state.big;
	start = heap._class->head;
	while (start != NULL) {
		if (ptr_in_heap(start, ptr)) {
			heap.addr = start;
			return heap;
		}
		start = mm_hdr_to_ftr(start)->next_heap;
	}
	heap.addr = NULL;
	return heap;
}

t_header *mm_grow_heap_class(t_heap_class *heap_class, size_t size)
{
	size = heap_class->size ? heap_class->size : size;
	t_header *head = mmap(heap_class->head, size, PROT_READ | PROT_WRITE,
			      MAP_PRIVATE | MAP_ANON, -1, 0);
	if (head == MAP_FAILED)
		return NULL;
	head->size = size;
	mm_set_hdr_mask(head);
	mm_hdr_to_ftr(head)->next_heap = heap_class->head;
	heap_class->head = head;

	t_header *blk = mm_hdr_to_pl(head);
	blk->size = mm_pl_size(head);
	mm_set_hdr_mask(blk);
	mm_set_free_bit(blk);
	*mm_hdr_to_ftr(blk) = *blk;
	mm_add_free_blk(heap_class, blk);

	return head;
}

bool mm_heap_empty(t_header *head)
{
	return mm_pl_size(head) == mm_blk_size(mm_hdr_to_pl(head));
}

static inline t_header *get_prev_heap(t_heap heap)
{
	t_header *prev = NULL;
	t_header *start = heap._class->head;

	while (start != heap.addr) {
		prev = start;
		start = mm_hdr_to_ftr(start)->next_heap;
	}
	return prev;
}

void mm_free_heap(t_heap heap)
{
	t_header *prev = get_prev_heap(heap);
	if (prev == NULL) {
		heap._class->head = mm_hdr_to_ftr(heap.addr)->next_heap;
	} else {
		mm_hdr_to_ftr(prev)->next_heap =
		    mm_hdr_to_ftr(heap.addr)->next_heap;
	}

	munmap(heap.addr, mm_blk_size(heap.addr));
}
