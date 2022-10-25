#include <libft.h>
#include "./ft_malloc.h"

void *ft_realloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return ft_malloc(size);

	if (size == 0)
	{
		ft_free(ptr);
		return NULL;
	}

	t_find_block_data data = find_block_data(ptr, g_data.tiny_pages);
	if (data.block == NULL)
		data = find_block_data(ptr, g_data.small_pages);
	if (data.block == NULL)
		data = find_block_data(ptr, g_data.large_pages);

	if (data.block == NULL)
		return NULL;

	t_block *block = data.block;
	t_page *page = data.page;

	if (size + sizeof(t_block) <= block->real_size)
	{
		block->real_size = size + sizeof(t_block);
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
		max_possible_block_size = ((char *)page->addr + page->real_size) - (char *)block;

	if (size + sizeof(t_block) <= max_possible_block_size)
	{
		block->real_size = size + sizeof(t_block);
		return block->addr;
	}


	// at this point we need to move the block to a new location
	void *new_ptr = ft_malloc(size);
	if (!new_ptr)
		return NULL;

	ft_memcpy(new_ptr, ptr, block->real_size);
	internal_free(data); // a bit more efficient since we already searched for block position
	return new_ptr;
}