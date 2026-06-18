#include "../includes/malloc.h"
#include "../internal/malloc_internal.h"
#include <errno.h>

void *calloc(size_t nmemb, size_t size)
{
	LOG_CALLOC_CALL(nmemb, size);
	if (size != 0 && nmemb > SIZE_MAX / size) {
		errno = ENOMEM;
		return NULL;
	}
	void *ptr = malloc(nmemb * size);
	if (ptr == NULL)
		return NULL;
	mm_memset(ptr, 0, nmemb * size);
	LOG_CALLOC_RET(ptr);
	return ptr;
}
