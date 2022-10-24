#include <libft.h>
#include "./ft_malloc.h"

void ft_free(void *ptr)
{
	if (ptr == NULL)
		return;

	t_page *prev_page = NULL;

	for (t_page *page = g_data.pages; page != NULL; page = page->next)
	{
		for (t_block *block = page->first; block != NULL; block = block->next)
		{
			if (block->addr == ptr)
			{
				int free_page = 0;

				// Block is the only block in the page => free the page
				if (page->first == block && page->last == block)
				{
					page->first = NULL;
					page->last = NULL;

					free_page = 1;

					// return;
				}

				// Block is the last block in the page
				if (block == page->last && block->prev)
				{
					// If previous block is in same page (blocks are always sorted by addr), then it's the new last block
					if (block->prev->addr >= page->addr && (char *)block->prev->addr < (char *)page->addr + page->size)
						page->last = block->prev;
					else
					{
						free_page = 1;
					}
				}

				// Block is the frist block in the page
				if (block == page->first && block->next)
				{
					if (block->next->addr >= page->addr && (char *)block->next->addr < (char *)page->addr + page->size)
						page->first = block->next;
					else
					{
						free_page = 1;
					}
				}

				// free block
				if (block->prev != NULL)
					block->prev->next = block->next;

				if (block->next != NULL)
					block->next->prev = block->prev;

				if (block == g_data.blocks)
					g_data.blocks = block->next;

				if (free_page)
				{
					if (munmap(page->addr, page->size) == -1)
						printf("munmap failed\n");

					if (prev_page)
						prev_page->next = page->next;
					else
						g_data.pages = page->next;

					t_meta_page *prev_meta_page = NULL;
					for (t_meta_page *meta_page = g_data.meta_pages; meta_page != NULL; meta_page = meta_page->next)
					{
						if (meta_page->first_page == page && meta_page->last_page == page)
						{
							if (prev_meta_page)
								prev_meta_page->next = meta_page->next;
							else // meta_page == g_data.meta_pages
								g_data.meta_pages = meta_page->next;

							if (munmap(meta_page->addr, meta_page->size) == -1)
								printf("munmap of meta page failed\n");

							break;
						}
						else if (meta_page->first_page == page)
						{
							meta_page->first_page = page->next;
							break;
						}
						else if (meta_page->last_page == page)
						{
							meta_page->last_page = prev_page;
							break;
						}

						prev_meta_page = meta_page;
					}
				}

				return;
			}

			if (block == page->last)
				break;
		}

		prev_page = page;
	}
}