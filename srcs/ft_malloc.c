#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#include "ft_malloc.h"

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
	page->alloc = NULL;
	page->free = NULL;
	page->next = NULL;
	page->size = alloc_size;

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

void *malloc(size_t size)
{
	t_page *new_page = allocate_page(1);

	printf("Page address is %p, size is %zu\n", new_page->addr, new_page->size);

	return NULL;
}