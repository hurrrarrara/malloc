#include <stdlib.h>
#include <string.h>
#include "../includes/malloc.h"

int main(void)
{
	char *p = malloc(1);

	for (size_t i = 1; i < 10000000; i *= 2) {
		p = realloc(p, i);
		if (!p)
			return 1;
		memset(p, 0, i);
		show_alloc_mem();
	}

	free(p);
}
