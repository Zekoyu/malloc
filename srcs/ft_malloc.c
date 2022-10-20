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

	page->addr = (char *)addr + sizeof(t_page);
	page->real_addr = addr;
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
		printf("%p - %p : %zu bytes\n", block->addr, block->addr + block->size, block->size);
		alloc_bytes += block->size;
	}

	printf("Total : %zu bytes\n", alloc_bytes);
}

typedef struct s_free_space
{
	t_page *page;
	t_block *prev_block;
	void *addr;
} t_free_space;

// find free space of size bytes in already allocated pages (called should count the t_block size as part of size)
t_free_space find_free_space(size_t size)
{
	t_free_space ret;

	for (t_page *page = g_page; page != NULL; page = page->next)
	{
		if (!page->first && size <= page->size) // page is empty
		{
			ret.addr = page->addr;
			ret.page = page;
			ret.prev_block = NULL;
			return ret;
		}

		for (t_block *block = page->first; block != page->last; block = block->next)
		{
			size_t free_space = block->next->real_addr - (block->real_addr + block->real_size);

			if (free_space >= size)
			{
				ret.addr = block->real_addr + block->real_size;
				ret.page = page;
				ret.prev_block = block;
				return ret;
			}
		}

		char *page_end = page->real_addr + page->real_size;
		char *last_block_end = page->last->real_addr + page->last->real_size;

		// while loop check in between blocks, not outside
		if (size <= page_end - last_block_end) // check after last block
		{
			ret.addr = last_block_end;
			ret.page = page;
			ret.prev_block = page->last;
			return ret;
		}

		char *page_start = page->addr;
		char *first_block_start = page->first->real_addr;

		if (size <= first_block_start - page_start) // check before first block (eg. block was freed so now there is place)
		{
			ret.addr = page_start;
			ret.page = page;
			ret.prev_block = NULL;
			return ret;
		}
	}

	ret.addr = NULL;
	ret.page = NULL;
	return ret;
}

t_block *get_last_block()
{
	printf("g_page first block is %p\n", g_page->first);
	t_block *last = NULL;
	for (t_block *block = g_page ? g_page->first : NULL; block != NULL; block = block->next)
		last = block;
	return last;
}

t_block *init_block(void *addr, size_t size)
{
	t_block *block = (t_block *)addr;
	block->addr = (char *)addr + sizeof(t_block);
	block->real_addr = addr;
	block->next = NULL;
	block->prev = NULL;
	block->size = size;
	block->real_size = size + sizeof(t_block);
	return block;
}

void *ft_malloc(size_t size)
{
	t_free_space free_space = find_free_space(size + sizeof(t_block));

	if (free_space.addr)
	{
		t_block *block = init_block(free_space.addr, size);
		t_page *page = free_space.page;

		if (free_space.prev_block)
		{
			t_block *next = free_space.prev_block->next;
			block->next = next;
			free_space.prev_block->next = block;
			if (free_space.prev_block == page->last) // same as if (next == NULL)
				page->last = block;
		}
		else
		{
			page->first = block;
			page->last = block;
		}
	}
	else
	{
		size_t page_count = size / 4096 + (size % 4096 ? 1 : 0); // round up
		t_page *page = allocate_page(page_count);
		if (!page)
		{
			printf("Cannot allocate %zu pages\n", page_count);
			return NULL;
		}
		t_block *last = get_last_block();
		printf("Last block is %p while allocating %zu bytes\n", last, size);
		t_block *block = init_block(page->addr, size);
		if (last)
		{
			last->next = block;
			block->prev = last;
		}
		page->first = block;
		page->last = block;
	}


	return NULL;
}