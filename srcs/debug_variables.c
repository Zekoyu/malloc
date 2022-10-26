#include <libft.h>
#include "./ft_malloc.h"

void verify_block_integrity_in_pages(t_page *pages)
{
	for (t_page *page = pages; page != NULL; page = page->next)
	{
		for (t_block *block = page->first; block != NULL; block = block->next)
		{
			if (!is_front_guard_intact(block))
				fprintf(stderr, "Front guard is not intact for address %p (size %zu)\n", block->addr, block->real_size - sizeof(t_block));
			if (!is_back_guard_intact(block))
				fprintf(stderr, "Back guard is not intact for address %p (size %zu)\n", block->addr, block->real_size - sizeof(t_block));
		}
	}
}


/**
 * @brief Go through all blocks and inspect the front / back guards
 * If one is invalid, print an error message on STDERR
 */
void verify_blocks_integrity()
{
	verify_block_integrity_in_pages(g_data.tiny_pages);
	verify_block_integrity_in_pages(g_data.small_pages);
	verify_block_integrity_in_pages(g_data.large_pages);
}