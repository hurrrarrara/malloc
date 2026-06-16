#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../includes/malloc.h"

void test_basic(void)
{
	void *p = malloc(64);
	assert(p != NULL);
	assert((uintptr_t)p % 16 == 0); // alignment check
	memset(p, 0xAB, 64);		// write pattern
	free(p);
	printf("basic: OK\n");
}

void test_split(void)
{
	void *p1 = malloc(64);
	void *p2 = malloc(64);
	assert(p1 != NULL && p2 != NULL);
	assert(p1 != p2);
	free(p1);
	free(p2);
	printf("split: OK\n");
}

void test_reuse(void)
{
	void *p1 = malloc(64);
	free(p1);
	void *p2 = malloc(64);
	assert(p1 == p2); // should reuse the same block
	free(p2);
	printf("reuse: OK\n");
}

void test_coalesce(void)
{
	void *p1 = malloc(64);
	void *p2 = malloc(64);
	void *p3 = malloc(64);
	show_alloc_mem();
	free(p1);
	free(p2); // should coalesce with p1
	void *p4 = malloc(128);
	show_alloc_mem();
	assert(p4 == p2); // reuses the coalesced block
	free(p3);
	free(p4);
	printf("coalesce: OK\n");
}

int main(void)
{
	// test_basic();
	// test_split();
	// test_reuse();
	test_coalesce();
}
