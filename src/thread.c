#include <pthread.h>

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void mm_lock_mtx()
{
	pthread_mutex_lock(&g_mutex);
}

void mm_unlock_mtx()
{
	pthread_mutex_unlock(&g_mutex);
}
