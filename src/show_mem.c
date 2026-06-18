#include "../internal/malloc_internal.h"

static void dump_block(t_header *block)
{
	size_t size = mm_blk_size(block);
	int is_alloc = !mm_has_free_bit(block);

	write_str("  [");
	is_alloc ? write_str(C_RED) : write_str(C_GREEN);
	write_ptr(block);
	write_str(C_RESET "] HDR     size=");
	write_nbr(size, 10);

	write_str("\n");

	if (is_alloc) {
		void *payload = mm_hdr_to_pl(block);

		write_str("  [");
		is_alloc ? write_str(C_RED) : write_str(C_GREEN);
		write_ptr(payload);
		write_str(C_RESET "] PAYLOAD size=");
		write_nbr(size - sizeof(t_header) * 2, 10);
		write_str("\n");
	} else {

		t_free_pl *free_pl = mm_hdr_to_pl(block);
		write_str("  [");
		is_alloc ? write_str(C_RED) : write_str(C_GREEN);
		write_ptr(mm_hdr_to_pl(block));
		write_str(C_RESET "] FREE    next=" C_GREEN);
		write_ptr(free_pl->next);
		write_str(C_RESET " | prev=" C_GREEN);
		write_ptr(free_pl->prev);
		write_str(C_RESET "\n");
	}

	t_header *ftr = mm_hdr_to_ftr(block);

	write_str("  [");
	!mm_has_free_bit(ftr) ? write_str(C_RED) : write_str(C_GREEN);
	write_ptr(ftr);
	write_str(C_RESET "] FTR     size=");
	write_nbr(mm_blk_size(ftr), 10);

	write_str("\n");
}

void dump_heap(void *head, int nb, char *heap_class)
{
	t_header *footer;
	size_t heap_size = mm_blk_size(head);

	if (nb > 0)
		write_str("\n--------------------------------------------------"
			  "\n");

	write_str("HEAP #");
	write_nbr(nb, 10);
	write_str(" @ " C_CYAN);
	write_ptr(head);
	write_str(C_RESET "  size = ");
	write_nbr(heap_size, 10);
	write_str(" bytes   [");
	write_str(heap_class);
	write_str("]\n\n");

	t_header *block = (t_header *)((char *)head + sizeof(t_header));
	t_header *end = mm_hdr_to_ftr(head);

	while (block < end) {
		dump_block(block);

		block = mm_next_hdr(block);
		write_str("\n");
	}

	footer = mm_hdr_to_ftr(head);

	write_str("HEAP FOOTER next_heap=" C_CYAN);
	if (footer->next_heap)
		write_ptr(footer->next_heap);
	else
		write_str("NULL");
	write_str(C_RESET);
}

static inline void show_alloc_mem_impl()
{
	write_str("\n================ ALLOCATOR STATE ================\n\n");
	write_nbr(TINY_PAGE_SIZE, 10);
	write_str(" | ");
	write_nbr(TINY_BLOCK_SIZE, 10);
	write_str("\n");
	write_nbr(SMALL_PAGE_SIZE, 10);
	write_str(" | ");
	write_nbr(SMALL_BLOCK_SIZE, 10);
	write_str("\n");
	if (g_state.tiny.head == NULL && g_state.small.head == NULL &&
	    g_state.big.head == NULL) {
		write(1, "HEAP : heap is empty\n", 21);
		write_str("\n\n================================================"
			  "=\n");
		return;
	}
	size_t i = 0;
	t_header *start = g_state.tiny.head;
	while (start != NULL) {
		dump_heap(start, i++, C_CYAN "TINY" C_RESET);
		start = mm_hdr_to_ftr(start)->next_heap;
	}
	i = 0;
	start = g_state.small.head;
	while (start != NULL) {
		dump_heap(start, i++, C_CYAN "SMALL" C_RESET);
		start = mm_hdr_to_ftr(start)->next_heap;
	}
	i = 0;
	start = g_state.big.head;
	while (start != NULL) {
		dump_heap(start, i++, C_CYAN "BIG" C_RESET);
		start = mm_hdr_to_ftr(start)->next_heap;
	}

	write_str("\n\n=================================================\n");
}

void show_alloc_mem()
{
	mm_lock_mtx();

	show_alloc_mem_impl();

	mm_unlock_mtx();
}
