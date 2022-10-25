#include <stdio.h>
#include <sys/mman.h>

#include "./ft_malloc.h"
#include "../libft/libft.h"

void *ft_malloc(size_t size)
{
	#if POUET_DEBUG
	printf("----- Malloc %zu bytes -----\n", size);
	#endif

	if (size == 0)
		return NULL;

	// Tiny malloc
	if (size <= TINY_FT_MALLOC_MAX_SIZE)
		return find_or_alloc_space(size, E_PAGE_TYPE_TINY);
	// Small malloc
	else if (size <= SMALL_FT_MALLOC_MAX_SIZE)
		return find_or_alloc_space(size, E_PAGE_TYPE_SMALL);
	//Large malloc
	else
		return find_or_alloc_space(size, E_PAGE_TYPE_LARGE);
}