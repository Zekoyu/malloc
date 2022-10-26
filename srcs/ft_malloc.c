#include <libft.h>
#include "./ft_malloc.h"

t_malloc_data g_data = { 0 };
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// Internal malloc without mutex_lock (used in realloc)
t_block *internal_malloc(size_t size)
{
	t_block *block = NULL;

	if (size <= TINY_FT_MALLOC_MAX_SIZE)
		block = allocate_block(size, &g_data.tiny_pages);
	else if (size <= SMALL_FT_MALLOC_MAX_SIZE)
		block = allocate_block(size, &g_data.small_pages);
	else
		block = allocate_block(size, &g_data.large_pages);

	return block;
}

void *ft_malloc(size_t size)
{
	if (size == 0)
		return NULL;

	pthread_mutex_lock(&g_mutex);

	#if defined(FT_MALLOC_BACK_GUARD) || defined(FT_MALLOC_FRONT_GUARD)
	verify_blocks_integrity();
	#endif

	t_block *block = internal_malloc(size);

	if (block)
	{

	}

	pthread_mutex_unlock(&g_mutex);

	if (block)
		return block->addr;

	return NULL;
}
