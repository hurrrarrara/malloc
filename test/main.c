#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../includes/malloc.h"

// int main(void) {
//   char *ptr = malloc(100);
//
//   if (!ptr) {
//     perror("malloc");
//     return 2;
//   }
//
//   strcpy(ptr, "Hello from custom malloc!\n\0");
//   write(1, ptr, 27);
//
//   show_alloc_mem();
//   write(1, "\n###############\n\0", 18);
//
//   char *a, *b;
//   a = NULL;
//   b = NULL;
//   for (int i = 0; i < 1000; i++) {
//     a = malloc(10 + (1 * 11 & 71));
//     if (b != NULL)
//       free(b);
//     b = NULL;
//     b = a;
//     a = NULL;
//     show_alloc_mem();
//     write(1, "\n###############\n\0", 18);
//   };
//   write(1, "out of for()\n\0", 14);
//   if (a != NULL)
//     free(a);
//   if (b != NULL)
//     free(b);
//
//   show_alloc_mem();
//   write(1, "\n###############\n\0", 18);
//
//   free(ptr);
//   // show_alloc_mem();
//   // write(1, "\n###############\n\0", 18);
//
//   return 0;
// }

#include <stdlib.h>
#include <string.h>

static inline void print_nb(size_t n, size_t base)
{
	static char c[] = "0123456789abcdef";

	if (n >= base)
		print_nb(n / base, base);
	write(1, c + (n % base), 1);
}

int main(void)
{
	const int count = 4;
	void *ptrs[count];
	for (int i = 0; i < count; i++) {
		ptrs[i] = malloc(9000);
		write(1, "allocation : ", 13);
		print_nb((uintptr_t)ptrs[i], 16);
		write(1, "\n", 1);
		// write(1, "\n###############\n\0", 18);
	}
	show_alloc_mem();
	ptrs[3] = realloc(ptrs[3], 9010);
	show_alloc_mem();

	// write(1, "\n###############\n\0", 18);
	// write(1, "\n\n\0", 3);
	// write(1, "\n###############\n\0", 18);
	//
	for (int i = 0; i < 4; i++) {
		free(ptrs[3 - i]);
		// show_alloc_mem();
		// write(1, "\n###############\n\0", 18);
	}
	return 0;
}

// int main(void)
// {
// 	void *ptrs[512];
// 	int i;
//
// 	for (i = 0; i < 1024; i++) {
// 		ptrs[i] = malloc((i % 32) + 18);
// 		// if (ptrs[i] == NULL)
// 		// 	break;
// 		// show_alloc_mem();
// 		// memset(ptrs[i], 'A', (i % 32) + 1);
// 	}
//
// 	show_alloc_mem();
// 	// for (int j = 0; j < 500; j += 2) {
// 	// 	free(ptrs[j]);
// 	// show_alloc_mem();
// 	// }
//
// 	// for (i = 0; i < 256; i++) {
// 	// 	void *p = malloc(8);
// 	// 	// memset(p, 'B', 8);
// 	// 	// show_alloc_mem();
// 	// 	free(p);
// 	// }
//
// 	for (i = 1; i < 1024; i += 2) {
//
// 		free(ptrs[i]);
// 	}
//
// 	return (0);
// }
