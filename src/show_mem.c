#include "../internal/malloc_internal.h"

static void dump_block(t_block *block)
{
	size_t size = block_size(block);
	int is_alloc = !is_free_block(block);

	write_str("  [");
	is_alloc ? write_str(C_RED) : write_str(C_GREEN);
	write_ptr(block);
	write_str(C_RESET "] HDR     size=");
	write_nbr(size, 10);

	write_str("\n");

	if (is_alloc) {
		void *payload = get_payload(block);

		write_str("  [");
		is_alloc ? write_str(C_RED) : write_str(C_GREEN);
		write_ptr(payload);
		write_str(C_RESET "] PAYLOAD size=");
		write_nbr(size - sizeof(t_block) * 2, 10);
		write_str("\n");
	} else {
		t_free_block *free_blk = (t_free_block *)block;
		write_str("  [");
		is_alloc ? write_str(C_RED) : write_str(C_GREEN);
		write_ptr(get_payload(block));
		write_str(C_RESET "] FREE    next=" C_GREEN);
		write_ptr(free_blk->next);
		write_str(C_RESET " | prev=" C_GREEN);
		write_ptr(free_blk->prev);
		write_str(C_RESET "\n");
	}

	t_block *ftr = get_footer(block);

	write_str("  [");
	!is_free_block(ftr) ? write_str(C_RED) : write_str(C_GREEN);
	write_ptr(ftr);
	write_str(C_RESET "] FTR     size=");
	write_nbr(block_size(ftr), 10);

	write_str("\n");
}

void dump_heap(void *head, int nb, char *heap_class)
{
	t_block *footer;
	size_t heap_size = block_size(head);

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

	t_block *block = (t_block *)((char *)head + sizeof(t_block));
	// t_block *end = get_footer(head);

	// while (block < end) {
	dump_block(block);

	block = next_block(block);
	write_str("\n");
	// }

	footer = get_footer(head);

	write_str("HEAP FOOTER next_heap=" C_CYAN);
	if (footer->next)
		write_ptr(footer->next);
	else
		write_str("NULL");
	write_str(C_RESET);
}

void show_alloc_mem()
{
	write_str("\n================ ALLOCATOR STATE ================\n\n");
	if (g_state.tiny.head == NULL && g_state.small.head == NULL &&
	    g_state.big.head == NULL) {
		write(1, "HEAP : heap is empty\n", 21);
		write_str("\n\n================================================"
			  "=\n");
		return;
	}
	size_t i = 0;
	t_block *start = g_state.tiny.head;
	while (start != NULL) {
		// dump_heap(start, i++, C_CYAN "TINY" C_RESET);
		start = get_footer(start)->next;
		write_str("T");
	}
	i = 0;
	start = g_state.small.head;
	while (start != NULL) {
		// dump_heap(start, i++, C_CYAN "SMALL" C_RESET);
		start = get_footer(start)->next;
		write_str("S");
	}
	i = 0;
	start = g_state.big.head;
	while (start != NULL) {
		dump_heap(start, i++, C_CYAN "BIG" C_RESET);
		start = get_footer(start)->next;
		write_str("B");
	}

	write_str("\n\n=================================================\n");
}
