#include <libft.h>
#include "./ft_malloc.h"

t_malloc_data g_data = { 0 };

void *ft_malloc(size_t size)
{
	if (size == 0)
		return NULL;

	t_block *block = NULL;

	if (size <= TINY_FT_MALLOC_MAX_SIZE)
		block = allocate_block(size, &g_data.tiny_pages);
	else if (size <= SMALL_FT_MALLOC_MAX_SIZE)
		block = allocate_block(size, &g_data.small_pages);
	else
		block = allocate_block(size, &g_data.large_pages);

	if (block)
		return block->addr;

	return NULL;
}

void ft_free(void *ptr)
{
	return;
}

void show_alloc_mem()
{
	return;
}