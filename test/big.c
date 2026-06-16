#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

/* ─────────────────────────────────────────────
   Helpers
   ───────────────────────────────────────────── */

#define ALIGNMENT 16
#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST(name)                                                             \
	do {                                                                   \
		g_tests_run++;                                                 \
		printf("  %-45s", name);                                       \
		fflush(stdout);                                                \
	} while (0)

#define PASS()                                                                 \
	do {                                                                   \
		g_tests_passed++;                                              \
		printf(COLOR_GREEN "PASS" COLOR_RESET "\n");                   \
	} while (0)

#define FAIL(msg)                                                              \
	do {                                                                   \
		g_tests_failed++;                                              \
		printf(COLOR_RED "FAIL" COLOR_RESET " (%s)\n", msg);           \
	} while (0)

#define SECTION(name)                                                          \
	printf("\n" COLOR_YELLOW "── %s ──" COLOR_RESET "\n", name)

static int is_aligned(void *ptr)
{
	return ((uintptr_t)ptr % ALIGNMENT) == 0;
}

/* Fill memory with a deterministic pattern based on pointer address */
static void fill_pattern(void *ptr, size_t size)
{
	unsigned char *p = ptr;
	unsigned char val = (unsigned char)((uintptr_t)ptr & 0xFF);
	for (size_t i = 0; i < size; i++)
		p[i] = (unsigned char)(val + i);
}

/* Return 1 if the pattern written by fill_pattern is intact */
static int check_pattern(void *ptr, size_t size)
{
	unsigned char *p = ptr;
	unsigned char val = (unsigned char)((uintptr_t)ptr & 0xFF);
	for (size_t i = 0; i < size; i++)
		if (p[i] != (unsigned char)(val + i))
			return 0;
	return 1;
}

/* ─────────────────────────────────────────────
   1. Basic correctness
   ───────────────────────────────────────────── */

static void test_null_malloc(void)
{
	TEST("malloc(0) returns NULL or valid ptr");
	void *p = malloc(0);
	/* Both NULL and a valid ptr are acceptable per the C standard */
	if (p)
		free(p);
	PASS();
}

static void test_non_null(void)
{
	TEST("malloc(1) returns non-NULL");
	void *p = malloc(1);
	if (!p) {
		FAIL("returned NULL");
		return;
	}
	free(p);
	PASS();
}

static void test_alignment_small(void)
{
	TEST("alignment: small allocations (1–15 bytes)");
	for (size_t s = 1; s <= 15; s++) {
		void *p = malloc(s);
		if (!p || !is_aligned(p)) {
			free(p);
			FAIL("misaligned");
			return;
		}
		free(p);
	}
	PASS();
}

static void test_alignment_various(void)
{
	TEST("alignment: powers of two up to 1MB");
	for (size_t s = 1; s <= (1 << 20); s <<= 1) {
		void *p = malloc(s);
		if (!p || !is_aligned(p)) {
			free(p);
			FAIL("misaligned");
			return;
		}
		free(p);
	}
	PASS();
}

static void test_write_and_read(void)
{
	TEST("write then read back (64 sizes)");
	for (size_t s = 1; s <= 4096; s *= 2) {
		void *p = malloc(s);
		if (!p) {
			FAIL("NULL");
			return;
		}
		fill_pattern(p, s);
		if (!check_pattern(p, s)) {
			free(p);
			FAIL("pattern mismatch");
			return;
		}
		free(p);
	}
	PASS();
}

static void test_free_null(void)
{
	TEST("free(NULL) does not crash");
	free(NULL);
	PASS();
}

/* ─────────────────────────────────────────────
   2. Reuse and splitting
   ───────────────────────────────────────────── */

static void test_reuse_exact(void)
{
	TEST("freed block is reused for same size");
	void *p1 = malloc(128);
	if (!p1) {
		FAIL("NULL");
		return;
	}
	free(p1);
	void *p2 = malloc(128);
	if (!p2) {
		FAIL("NULL");
		return;
	}
	/* A correct allocator should hand back the same block */
	if (p1 != p2) {
		free(p2);
		FAIL("block not reused");
		return;
	}
	free(p2);
	PASS();
}

static void test_split(void)
{
	TEST("large free block is split for smaller request");
	void *big = malloc(1024);
	if (!big) {
		FAIL("NULL");
		return;
	}
	free(big);

	void *a = malloc(64);
	void *b = malloc(64);
	if (!a || !b) {
		free(a);
		free(b);
		FAIL("NULL");
		return;
	}
	if (a == b) {
		free(a);
		free(b);
		FAIL("same ptr");
		return;
	}
	free(a);
	free(b);
	PASS();
}

static void test_no_overlap(void)
{
	TEST("adjacent allocations do not overlap");
	void *p1 = malloc(64);
	void *p2 = malloc(64);
	if (!p1 || !p2) {
		free(p1);
		free(p2);
		FAIL("NULL");
		return;
	}

	fill_pattern(p1, 64);
	fill_pattern(p2, 64);

	if (!check_pattern(p1, 64) || !check_pattern(p2, 64)) {
		free(p1);
		free(p2);
		FAIL("overlap detected");
		return;
	}
	free(p1);
	free(p2);
	PASS();
}

/* ─────────────────────────────────────────────
   3. Coalescing
   ───────────────────────────────────────────── */

static void test_coalesce_next(void)
{
	TEST("coalesce: free + free-next → merged block reused");
	void *a = malloc(128);
	void *b = malloc(128);
	void *c = malloc(128); /* keep c alive so page isn't munmap'd */
	if (!a || !b || !c) {
		free(a);
		free(b);
		free(c);
		FAIL("NULL");
		return;
	}

	free(a);
	free(b); /* should coalesce with a */

	void *d = malloc(256);
	if (!d) {
		free(c);
		FAIL("NULL after coalesce");
		return;
	}
	/* d should sit at a's address (carved from merged a+b) */
	if (d != b) {
		free(d);
		free(c);
		FAIL("merged block not reused");
		return;
	}

	free(d);
	free(c);
	PASS();
}

static void test_coalesce_prev(void)
{
	TEST("coalesce: free-prev + free → merged block reused");
	void *a = malloc(128);
	void *b = malloc(128);
	void *c = malloc(128);
	if (!a || !b || !c) {
		free(a);
		free(b);
		free(c);
		FAIL("NULL");
		return;
	}

	free(b);
	free(a); /* should coalesce backwards with b */

	void *d = malloc(256);
	if (!d) {
		free(c);
		FAIL("NULL after coalesce");
		return;
	}
	if (d != b) {
		free(d);
		free(c);
		FAIL("merged block not reused");
		return;
	}

	free(d);
	free(c);
	PASS();
}

static void test_coalesce_both(void)
{
	TEST("coalesce: free-prev + free + free-next → full merge");
	void *a = malloc(128);
	void *b = malloc(128);
	void *c = malloc(128);
	void *d = malloc(128); /* anchor */
	if (!a || !b || !c || !d) {
		free(a);
		free(b);
		free(c);
		free(d);
		FAIL("NULL");
		return;
	}

	free(a);
	free(c);
	free(b); /* should coalesce with both a and c */

	void *e = malloc(384);
	if (!e) {
		free(d);
		FAIL("NULL after full coalesce");
		return;
	}
	if (e != c) {
		free(e);
		free(d);
		FAIL("merged block not reused");
		return;
	}

	free(e);
	free(d);
	PASS();
}

/* ─────────────────────────────────────────────
   4. Size classes
   ───────────────────────────────────────────── */

static void test_tiny_allocations(void)
{
	TEST("100 tiny allocations (1–16 bytes) and free");
	void *ptrs[100];
	for (int i = 0; i < 100; i++) {
		size_t s = (size_t)(i % 16) + 1;
		ptrs[i] = malloc(s);
		if (!ptrs[i] || !is_aligned(ptrs[i])) {
			for (int j = 0; j < i; j++)
				free(ptrs[j]);
			FAIL("NULL or misaligned");
			return;
		}
		fill_pattern(ptrs[i], s);
	}
	for (int i = 0; i < 100; i++)
		free(ptrs[i]);
	PASS();
}

static void test_medium_allocations(void)
{
	TEST("50 medium allocations (64–4096 bytes) and free");
	void *ptrs[50];
	size_t sizes[50];
	for (int i = 0; i < 50; i++) {
		sizes[i] = (size_t)(64 << (i % 7)); /* 64, 128, 256 … 4096 */
		ptrs[i] = malloc(sizes[i]);
		if (!ptrs[i] || !is_aligned(ptrs[i])) {
			for (int j = 0; j < i; j++)
				free(ptrs[j]);
			FAIL("NULL or misaligned");
			return;
		}
		fill_pattern(ptrs[i], sizes[i]);
	}
	/* Verify patterns before freeing */
	for (int i = 0; i < 50; i++)
		if (!check_pattern(ptrs[i], sizes[i])) {
			for (int j = 0; j < 50; j++)
				free(ptrs[j]);
			FAIL("pattern corrupted");
			return;
		}
	for (int i = 0; i < 50; i++)
		free(ptrs[i]);
	PASS();
}

static void test_large_allocations(void)
{
	TEST("10 large allocations (128KB–1MB) and free");
	void *ptrs[10];
	size_t sizes[10];
	for (int i = 0; i < 10; i++) {
		sizes[i] = (size_t)(128 * 1024) << i; /* 128K … up to ~65MB
							 total */
		ptrs[i] = malloc(sizes[i]);
		if (!ptrs[i] || !is_aligned(ptrs[i])) {
			for (int j = 0; j < i; j++)
				free(ptrs[j]);
			FAIL("NULL or misaligned");
			return;
		}
		memset(ptrs[i], 0xCD, sizes[i]); /* write every byte */
	}
	for (int i = 0; i < 10; i++)
		free(ptrs[i]);
	PASS();
}

/* ─────────────────────────────────────────────
   5. Interleaved alloc / free patterns
   ───────────────────────────────────────────── */

static void test_interleaved_alloc_free(void)
{
	TEST("interleaved alloc/free (LIFO pattern)");
#define N 256
	void *ptrs[N];
	size_t sizes[N];

	for (int i = 0; i < N; i++) {
		sizes[i] = (size_t)((i * 37 % 512) + 1);
		ptrs[i] = malloc(sizes[i]);
		if (!ptrs[i]) {
			for (int j = 0; j < i; j++)
				free(ptrs[j]);
			FAIL("NULL");
			return;
		}
		fill_pattern(ptrs[i], sizes[i]);
	}
	/* Free every other block */
	for (int i = 0; i < N; i += 2) {
		free(ptrs[i]);
		ptrs[i] = NULL;
	}
	/* Verify remaining blocks are intact */
	for (int i = 1; i < N; i += 2)
		if (!check_pattern(ptrs[i], sizes[i])) {
			for (int j = 1; j < N; j += 2)
				free(ptrs[j]);
			FAIL("pattern corrupted after partial free");
			return;
		}
	/* Reallocate the freed slots */
	for (int i = 0; i < N; i += 2) {
		ptrs[i] = malloc(sizes[i]);
		if (!ptrs[i]) {
			for (int j = 0; j < N; j++)
				free(ptrs[j]);
			FAIL("NULL on realloc");
			return;
		}
	}
	for (int i = 0; i < N; i++)
		free(ptrs[i]);
#undef N
	PASS();
}

static void test_fifo_pattern(void)
{
	TEST("FIFO pattern: alloc 1, free 1, repeat (512 iterations)");
	for (int i = 0; i < 512; i++) {
		size_t s = (size_t)((i % 256) + 1);
		void *p = malloc(s);
		if (!p) {
			FAIL("NULL");
			return;
		}
		memset(p, i & 0xFF, s);
		free(p);
	}
	PASS();
}

static void test_checkerboard_free(void)
{
	TEST("checkerboard free then re-alloc");
#define M 64
	void *ptrs[M];
	for (int i = 0; i < M; i++) {
		ptrs[i] = malloc(128);
		if (!ptrs[i]) {
			for (int j = 0; j < i; j++)
				free(ptrs[j]);
			FAIL("NULL");
			return;
		}
	}
	/* Free even indices */
	for (int i = 0; i < M; i += 2) {
		free(ptrs[i]);
		ptrs[i] = NULL;
	}
	/* Free odd indices */
	for (int i = 1; i < M; i += 2) {
		free(ptrs[i]);
		ptrs[i] = NULL;
	}
	/* All freed — one big realloc should succeed */
	void *big = malloc(128 * M / 2);
	if (!big) {
		FAIL("NULL after full coalesce");
		return;
	}
	free(big);
#undef M
	PASS();
}

/* ─────────────────────────────────────────────
   6. Stress test
   ───────────────────────────────────────────── */

static void test_stress_random(void)
{
	TEST("stress: 10 000 random alloc/free ops");
#define SLOTS 128
	void *ptrs[SLOTS] = {0};
	size_t sizes[SLOTS] = {0};

	srand(42);
	int ops = 10000;
	while (ops--) {
		int slot = rand() % SLOTS;
		if (ptrs[slot]) {
			/* Verify pattern before freeing */
			if (!check_pattern(ptrs[slot], sizes[slot])) {
				/* Free everything and report */
				for (int i = 0; i < SLOTS; i++)
					free(ptrs[i]);
				FAIL("pattern corrupted under stress");
				return;
			}
			free(ptrs[slot]);
			ptrs[slot] = NULL;
			sizes[slot] = 0;
		} else {
			sizes[slot] = (size_t)(rand() % 4096) + 1;
			ptrs[slot] = malloc(sizes[slot]);
			if (!ptrs[slot]) {
				sizes[slot] = 0;
				continue;
			}
			if (!is_aligned(ptrs[slot])) {
				for (int i = 0; i < SLOTS; i++)
					free(ptrs[i]);
				FAIL("misaligned under stress");
				return;
			}
			fill_pattern(ptrs[slot], sizes[slot]);
		}
	}
	for (int i = 0; i < SLOTS; i++)
		free(ptrs[i]);
#undef SLOTS
	PASS();
}

static void test_stress_mixed_sizes(void)
{
	TEST("stress: mixed tiny/medium/large with pattern check");
#define SLOTS 64
	void *ptrs[SLOTS] = {0};
	size_t sizes[SLOTS] = {0};
	/* Cycle through three size classes */
	size_t class_sizes[] = {8,   16,   32,	 64,	128,
				512, 1024, 4096, 65536, 131072};
	int nclasses = (int)(sizeof(class_sizes) / sizeof(class_sizes[0]));

	srand(1337);
	for (int round = 0; round < 200; round++) {
		for (int i = 0; i < SLOTS; i++) {
			if (ptrs[i]) {
				free(ptrs[i]);
				ptrs[i] = NULL;
			}
			sizes[i] = class_sizes[rand() % nclasses];
			ptrs[i] = malloc(sizes[i]);
			if (!ptrs[i]) {
				sizes[i] = 0;
				continue;
			}
			fill_pattern(ptrs[i], sizes[i]);
		}
	}
	for (int i = 0; i < SLOTS; i++)
		free(ptrs[i]);
#undef SLOTS
	PASS();
}

/* ─────────────────────────────────────────────
   7. Edge cases
   ───────────────────────────────────────────── */

static void test_many_small_then_one_large(void)
{
	TEST("many small allocs freed → one large alloc succeeds");
#define K 512
	void *ptrs[K];
	for (int i = 0; i < K; i++) {
		ptrs[i] = malloc(32);
		if (!ptrs[i]) {
			for (int j = 0; j < i; j++)
				free(ptrs[j]);
			FAIL("NULL");
			return;
		}
	}
	for (int i = 0; i < K; i++)
		free(ptrs[i]);

	/* After freeing all, a large block should be serviced */
	void *big = malloc(32 * K / 4);
	if (!big) {
		FAIL("NULL — fragmented heap?");
		return;
	}
	free(big);
#undef K
	PASS();
}

static void test_double_free_guard(void)
{
	TEST("double free does not crash (guarded allocator)");
	void *p = malloc(64);
	if (!p) {
		FAIL("NULL");
		return;
	}
	free(p);
	free(p); /* second free — your guard should catch this */
	PASS();
}

static void test_single_byte_many_times(void)
{
	TEST("malloc(1) × 1000, all freed");
#define L 1000
	void *ptrs[L];
	for (int i = 0; i < L; i++) {
		ptrs[i] = malloc(1);
		if (!ptrs[i] || !is_aligned(ptrs[i])) {
			for (int j = 0; j < i; j++)
				free(ptrs[j]);
			FAIL("NULL or misaligned");
			return;
		}
		*(char *)ptrs[i] = (char)i;
	}
	for (int i = 0; i < L; i++)
		if (*(char *)ptrs[i] != (char)i) {
			for (int j = 0; j < L; j++)
				free(ptrs[j]);
			FAIL("byte corrupted");
			return;
		}
	for (int i = 0; i < L; i++)
		free(ptrs[i]);
#undef L
	PASS();
}

static void test_page_boundary_sizes(void)
{
	TEST("allocations just below/at/above page size (4096)");
	size_t sizes[] = {4095, 4096, 4097, 8191, 8192, 8193};
	int n = (int)(sizeof(sizes) / sizeof(sizes[0]));
	for (int i = 0; i < n; i++) {
		void *p = malloc(sizes[i]);
		if (!p || !is_aligned(p)) {
			free(p);
			FAIL("NULL or misaligned at page boundary");
			return;
		}
		memset(p, 0xBE, sizes[i]);
		free(p);
	}
	PASS();
}

/* ─────────────────────────────────────────────
   8. munmap / page return
   ───────────────────────────────────────────── */

static void test_munmap_on_full_free(void)
{
	TEST("single large alloc freed → page returned to OS");
	/*
	 * We can't directly verify munmap was called, but we can check
	 * that a subsequent large alloc still works (OS gave us fresh pages).
	 */
	void *p1 = malloc(1024 * 1024);
	if (!p1) {
		FAIL("NULL");
		return;
	}
	free(p1);

	void *p2 = malloc(1024 * 1024);
	if (!p2) {
		FAIL("NULL after munmap");
		return;
	}
	free(p2);
	PASS();
}

/* ─────────────────────────────────────────────
   Runner
   ───────────────────────────────────────────── */

int main(void)
{
	printf("\n╔══════════════════════════════════════════════════╗\n");
	printf("║           malloc / free test suite               ║\n");
	printf("╚══════════════════════════════════════════════════╝\n");

	SECTION("1. Basic correctness");
	test_null_malloc();
	test_non_null();
	test_alignment_small();
	test_alignment_various();
	test_write_and_read();
	test_free_null();

	SECTION("2. Reuse and splitting");
	test_reuse_exact();
	test_split();
	test_no_overlap();

	SECTION("3. Coalescing");
	test_coalesce_next();
	test_coalesce_prev();
	test_coalesce_both();

	SECTION("4. Size classes");
	test_tiny_allocations();
	test_medium_allocations();
	test_large_allocations();

	SECTION("5. Interleaved alloc/free patterns");
	test_interleaved_alloc_free();
	test_fifo_pattern();
	test_checkerboard_free();

	SECTION("6. Stress");
	test_stress_random();
	test_stress_mixed_sizes();

	SECTION("7. Edge cases");
	test_many_small_then_one_large();
	test_double_free_guard();
	test_single_byte_many_times();
	test_page_boundary_sizes();

	SECTION("8. munmap / page return");
	test_munmap_on_full_free();

	/* ── Summary ── */
	printf("\n──────────────────────────────────────────────────\n");
	printf("  Total:  %d\n", g_tests_run);
	printf("  " COLOR_GREEN "Passed: %d" COLOR_RESET "\n", g_tests_passed);
	if (g_tests_failed)
		printf("  " COLOR_RED "Failed: %d" COLOR_RESET "\n",
		       g_tests_failed);
	else
		printf("  Failed: 0\n");
	printf("──────────────────────────────────────────────────\n\n");

	return g_tests_failed ? 1 : 0;
}
