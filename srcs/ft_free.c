#include <libft.h>
#include "./ft_malloc.h"

void ft_free(void *ptr)
{
	if (ptr == NULL)
		return;

	for (t_page *page = g_data.pages; page != NULL; page = page->next)
	{
		for (t_block *block = page->first; block != NULL; block = block->next)
		{
			if (block->addr == ptr)
			{
				// Block is the only block in the page
				if (page->first == block && page->last == block)
				{
					page->first = NULL;
					page->last = NULL;
				}

				// Block is the first block in the page


				if (page->last == block)
					page->last = block->prev;

				// free block
				if (block->prev != NULL)
					block->prev->next = block->next;
				else
					page->first = block->next;

				if (block->next != NULL)
					block->next->prev = block->prev;
				else
					page->last = block->prev;

				if (page->first == NULL)
				{
					if (page->prev != NULL)
						page->prev->next = page->next;
					else
						g_data.pages = page->next;

					if (page->next != NULL)
						page->next->prev = page->prev;
				}
			}

			if (block == page->last)
				break;
		}
	}
}