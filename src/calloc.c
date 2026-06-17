#include "../includes/malloc.h"
#include "../internal/malloc_internal.h"

void *calloc(size_t count, size_t size)
{
	if (size != 0 && count > SIZE_MAX / size) {
		// errno = ENOMEM;
		return NULL;
	}
	LOG("calloc");
	void *ptr = malloc(count * size);
	if (ptr == NULL)
		return NULL;
	for (size_t i = 0; i < count * size; i++)
		((char *)ptr)[i] = 0;
	return ptr;
}
