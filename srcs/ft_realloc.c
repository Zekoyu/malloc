#include <libft.h>
#include "./ft_malloc.h"

int get_size_category(size_t size)
{
	if (size <= TINY_FT_MALLOC_MAX_SIZE)
		return 0;
	else if (size <= SMALL_FT_MALLOC_MAX_SIZE)
		return 1;
	else
		return 2;
}

/**
 * @brief Checks if the given (block / page) is in the same category as the new size
 * (eg. malloc of 100 then realloc of 2000 is not in the same category, so change category)
 *
 * @param data the data for the given (block / page)
 * @param new_size the target size
 * @return int 0/1 whether it's in the same category or not
 */
int is_same_size_category(t_find_block_data data, size_t new_size)
{
	if (!data.block || !data.page)
		return (0);

	size_t original_size = data.block->real_size - sizeof(t_block);
	int original_category = get_size_category(original_size);
	int new_category = get_size_category(new_size);

	return (original_category == new_category);
}

void *realloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return malloc(size);

	if (size == 0)
	{
		free(ptr);
		return NULL;
	}

	pthread_mutex_lock(&g_mutex);

	#if defined(FT_MALLOC_BACK_GUARD) || defined(FT_MALLOC_FRONT_GUARD)
	verify_blocks_integrity();
	#endif

	t_find_block_data data = find_block_data(ptr, g_data.tiny_pages);
	if (data.block == NULL)
		data = find_block_data(ptr, g_data.small_pages);
	if (data.block == NULL)
		data = find_block_data(ptr, g_data.large_pages);

	if (data.block == NULL)
	{
		pthread_mutex_unlock(&g_mutex);
		return NULL;
	}

	size_t original_size = data.block->real_size - sizeof(t_block);
	// If we realloc from 1000 to 10, only move 10 first bytes, always take smaller size
	size_t move_size = original_size > size ? size : original_size;

	if (!is_same_size_category(data, size))
	{
		t_block *new_block = internal_malloc(size);
		if (new_block == NULL)
		{
			pthread_mutex_unlock(&g_mutex);
			return NULL;
		}

		ft_memcpy(new_block->addr, ptr, move_size);
		internal_free(data);
		pthread_mutex_unlock(&g_mutex);
		return new_block->addr;
	}

	t_block *block = data.block;
	t_page *page = data.page;

	if (size <= original_size)
	{
		block->real_size = size + sizeof(t_block);
		pthread_mutex_unlock(&g_mutex);
		return block->addr; // same as return ptr
	}

	// at this point the requested size is bigger than the current block size
	// so we need to check if we can extend the block to the next block or if we need to move it
	size_t max_possible_block_size = 0;
	if (block->next && block != page->last)
		// block == block->addr - sizeof(t_block)
		max_possible_block_size = (char *)block->next - (char *)block;
	else
		// page->addr is the same as page
		max_possible_block_size = ((char *)page + page->real_size) - (char *)block;

	if (size + sizeof(t_block) <= max_possible_block_size)
	{
		block->real_size = size + sizeof(t_block);
		pthread_mutex_unlock(&g_mutex);
		return block->addr;
	}


	// at this point we need to move the block to a new location
	// Use internal malloc instead of normal malloc to avoid mutex lock loop
	t_block *new_block = internal_malloc(size);
	if (!new_block)
	{
		pthread_mutex_unlock(&g_mutex);
		return NULL;
	}

	ft_memcpy(new_block->addr, ptr, move_size);
	internal_free(data); // a bit more efficient since we already searched for block position

	pthread_mutex_unlock(&g_mutex);
	return new_block->addr;
}