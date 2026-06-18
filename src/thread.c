#include <pthread.h>
#include "../internal/malloc_internal.h"

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

t_malloc_state g_state = {0};

void mm_lock_mtx()
{
	pthread_mutex_lock(&g_mutex);
}

void mm_unlock_mtx()
{
	pthread_mutex_unlock(&g_mutex);
}
