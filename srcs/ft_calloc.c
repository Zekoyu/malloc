#include "./ft_malloc.h"
#include <libft.h>

void *ft_calloc(size_t count, size_t size)
{
	void *ptr;

	ptr = ft_malloc(count * size);
	if (ptr)
		ft_bzero(ptr, count * size);
	return (ptr);
}