#include "./ft_malloc.h"
#include <libft.h>

void *ft_realloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return ft_malloc(size);

	if (size == 0)
	{
		ft_free(ptr);
		return NULL;
	}

	for (t_block *block = g_data.blocks; block != NULL; block = block->next)
	{
		if (block->addr == ptr)
		{
			if (block->size >= size)
			{
				block->size = size;
				return ptr;
			}
			else
			{
				void *new_ptr = ft_malloc(size);
				ft_memcpy(new_ptr, ptr, block->size);
				ft_free(ptr);
				return new_ptr;
			}
		}
	}

	return NULL;
}