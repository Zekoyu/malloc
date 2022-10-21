#include "./ft_malloc.h"
#include <libft.h>

size_t g_mmap_data_count = 0;

t_page *allocate_page(size_t count)
{
	void *addr;
	size_t alloc_size = count * getpagesize();

	if ((addr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED) // we dont care about last 2 args(fd and offset)
		return NULL;

	t_page *page;
	if ((page = create_page_metadata()) == NULL)
	{
		munmap(addr, alloc_size);
		return NULL;
	}

	g_mmap_data_count += count;

	page->addr = addr;
	page->size = alloc_size;

	return page;
}

size_t get_mmap_data_count()
{
	return g_mmap_data_count;
}

typedef struct s_free_space
{
	t_page *page;
	void *addr;
} t_free_space;

t_free_space find_free_space(size_t size)
{
	t_free_space free_data_space = { 0 };

	for (t_page *page = g_data.pages; page != NULL; page = page->next)
	{
		char free_space[page->size];
		ft_memset(free_space, 1, page->size);

		for (t_block *block = page->first; block != NULL; block = block->next)
		{
			size_t offset = (char *)block->addr - (char *)page->addr;
			if (offset <= page->size )// should be unnecessary but we never check enough
				ft_memset(free_space + offset, 0, block->size);

			if (block == page->last)
				break;
		}

		// printf("[");
		// for (size_t i = 0; i < page->size; i++)
		// {
		// 	printf("%c", free_space[i] + '0');
		// }
		// printf("]\n");


		size_t matched = 0;
		for (size_t i = 0; i < page->size; i++)
		{
			if (free_space[i] == 1)
			{
				matched++;
				if (matched >= size)
				{
					free_data_space.addr = (char *)page->addr + i + 1 - matched; // + 1 since i starts at 0
					free_data_space.page = page;
					return free_data_space;
				}
			}
			else
				matched = 0;
		}
	}

	return free_data_space;
}

t_block *allocate_block(void *data_addr, size_t block_size, t_page *page)
{
	#if POUET_DEBUG
	printf("Allocate block at %p\n", data_addr);
	#endif

	t_block *block;

	if ((block = create_block_metadata(page)) == NULL)
		return NULL;

	block->addr = data_addr;
	block->size = block_size;

	return block;
}

void *find_or_alloc_space(size_t size)
{
	t_free_space free_space = find_free_space(size);
	t_block *block = NULL;

	if (free_space.addr)
	{
		#if POUET_DEBUG
		printf("There is already space for %zu bytes malloc at %p\n", size, free_space.addr);
		#endif

		block = allocate_block(free_space.addr, size, free_space.page);
		if (!block)
			return NULL;
	}
	else
	{
		#if POUET_DEBUG
		printf("There is no space for %zu bytes malloc, allocating now\n", size);
		#endif

		size_t page_count = size / 4096 + (size % 4096 ? 1 : 0); // round up
	 	t_page *page = allocate_page(page_count);

		if (!page)
		{
			printf("Cannot allocate %zu pages\n", page_count);
			return NULL;
		}

		#if POUET_DEBUG
		printf("Putting block of %zu bytes at %p\n", size, page->addr);
		#endif

		block = allocate_block(page->addr, size, page);
		if (!block)
			return NULL;
	}

	if (!block)
		return NULL;

	return (block->addr);
}


