#include <stddef.h>
#include <stdint.h>

typedef uint32_t __attribute__((__may_alias__)) u32;
typedef uint64_t __attribute__((__may_alias__)) u64;

void *mm_memcpy(void *d, const void *s, size_t n)
{
	if (d == s)
		return d;
	while (n--)
		*((unsigned char *)d + n) = *((unsigned char *)s + n);
	return d;
}

void *mm_memmove(void *d, const void *s, size_t n)
{
	size_t i = -1;
	if (d == s)
		return d;
	if (d < s)
		while (++i < n)
			*(unsigned char *)(d + i) = *(unsigned char *)(s + i);
	else
		while (n--)
			*((unsigned char *)d + n) = *((unsigned char *)s + n);
	return d;
}

void *mm_memset(void *ptr, int value, size_t count)
{
	unsigned char *p = ptr;

	unsigned char v = value;
	u32 v32 = ((u32)-1) / 255 * (unsigned char)value;
	u64 v64 = v32 | ((u64)v32 << 32);

	if (!count)
		return p;
	size_t l = 0;
	for (; l < (count & ~(sizeof(u64) - 1)); l += sizeof(u64)) {
		*(u64 *)(p + l) = v64;
	}
	for (; l < (count & ~(sizeof(u32) - 1)); l += sizeof(u32)) {
		*(u32 *)(p + l) = v32;
	}
	for (; l < count; l++) {
		*(p + l) = v;
	}
	return p;
}
