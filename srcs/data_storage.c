#include "./ft_malloc.h"
#include <libft.h>

size_t g_mmap_data_count = 0;

t_page *allocate_page(size_t count, enum e_page_type type)
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
	page->type = type;

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

t_free_space find_free_space(size_t size, enum e_page_type type)
{
	t_free_space free_data_space = { 0 };

	for (t_page *page = g_data.pages; page != NULL; page = page->next)
	{
		if (page->type != type)
			continue;

		// - Check if no blocks
		// - Check before first block
		// - Check in between blocks
		// - Check after last block

		if (!page->first && size <= page->size)
		{
			free_data_space.addr = page->addr;
			free_data_space.page = page;
			return free_data_space;
		}

		size_t free_before_first_block = ((char *)page->first->addr - (char *)page->addr);
		if (page->first != NULL && free_before_first_block >= size)
		{
			free_data_space.addr = page->addr;
			free_data_space.page = page;
			return free_data_space;
		}

		for (t_block *block = page->first; block->next != NULL; block = block->next)
		{
			size_t free_space = (char *)block->next->addr - ((char *)block->addr + block->size);

			if (free_space >= size && block->next->addr >= page->addr && (char *)block->next->addr + block->next->size < (char *)page->addr + page->size)
			{
				free_data_space.addr = (char *)block->addr + block->size;
				free_data_space.page = page;
				return free_data_space;
			}

			if (block->next == page->last)
				break;
		}

		size_t free_after_last_block = ((char *)page->addr + page->size) - ((char *)page->last->addr + page->last->size);
		if (free_after_last_block >= size)
		{
			free_data_space.addr = (char *)page->last->addr + page->last->size;
			free_data_space.page = page;
			return free_data_space;
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

void *find_or_alloc_space(size_t size, enum e_page_type type)
{
	t_free_space free_space = find_free_space(size, type);
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

		size_t page_count;
		int page_size = getpagesize();

		if (type == E_PAGE_TYPE_TINY)
		{
			size_t alloc_size = TINY_FT_MALLOC_MAX_SIZE * 100;
			page_count = alloc_size / page_size + (alloc_size % page_size ? 1 : 0); // round up
		}
		else if (type == E_PAGE_TYPE_SMALL)
		{
			size_t alloc_size = SMALL_FT_MALLOC_MAX_SIZE * 100;
			page_count = alloc_size / page_size + (alloc_size % page_size ? 1 : 0);
		}
		else
		{
			page_count = size / page_size + (size % page_size ? 1 : 0);
		}

	 	t_page *page = allocate_page(page_count, type);

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

	return (block->addr);
}


