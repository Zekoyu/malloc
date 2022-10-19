#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#include "./ft_malloc.h"
#include "../libft/libft.h"

t_page *g_page = NULL;

t_page *allocate_page(size_t count)
{
	void *addr;
	size_t alloc_size = count * getpagesize();
	// allow read/write, private (only this process can see it), anonymous means it's not backed by a file, just for data storage
	if ((addr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED) // we dont care about last 2 args(fd and offset)
		return NULL;

	t_page *page = (t_page *)addr;

	page->addr = addr;
	page->first = NULL;
	page->last = NULL;
	page->next = NULL;
	page->real_size = alloc_size;
	page->size = alloc_size - sizeof(t_page); // available size

	if (!g_page)
		g_page = page;
	else
	{
		t_page *tmp = g_page;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = page;
	}

	return page;
}

void show_alloc_mem()
{
	size_t alloc_bytes = 0;
	for (t_block *block = g_page ? g_page->first : NULL; block != NULL; block = block->next)
	{
		ft_printf("%X - %X : %u bytes\n", block->addr, (char *)(block->addr) + block->size, block->size);
		alloc_bytes += block->size;
	}

	ft_printf("Total : %u bytes\n", alloc_bytes);
}

// find free space of size bytes in already allocated pages (called should count the t_block size as part of size)
void *find_free_space(size_t size)
{
	for (t_page *page = g_page; page != NULL; page = page->next)
	{
		for (t_block *block = page->first; block != page->last; block = block->next)
		{
			size_t free_space = block->next->addr - (block->addr + block->real_size);

			if (free_space >= size)
			{
				return block->addr + block->real_size;
			}
		}
	}

	return NULL;
}

void *ft_malloc(size_t size)
{
	t_page *new_page = allocate_page(1);

	printf("Page address is %p, size is %zu\n", new_page->addr, new_page->size);

	char *addr_start = ((char *)new_page->addr) + sizeof(t_page);

	printf("Address content: %s\n", addr_start);
	ft_strlcpy(addr_start, "Coucou delphin ca va ?", new_page->size - sizeof(t_page));
	printf("Address content: %s\n", addr_start);

	return NULL;
}