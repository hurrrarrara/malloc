#include "../internal/malloc_internal.h"

void free(void *ptr)
{
	if (ptr == NULL || !IS_ALIGNED((uintptr_t)ptr))
		return;
}
