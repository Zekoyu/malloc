#include "./ft_malloc.h"
#include "../libft/libft.h"

t_malloc_data g_data = { 0 };


t_meta_page *allocate_meta_page(size_t count)
{
	void *addr;
	size_t alloc_size = count * getpagesize();
	// allow read/write, private (only this process can see it), anonymous means it's not backed by a file, just for data storage
	if ((addr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED) // we dont care about last 2 args(fd and offset)
		return NULL;

	t_meta_page *meta_page = (t_meta_page *)addr;

	meta_page->addr = addr;
	meta_page->first_page = NULL;
	meta_page->last_page = NULL;
	meta_page->next = NULL;
	meta_page->size = alloc_size;

	if (!g_data.meta_pages)
	{
		g_data.meta_pages = meta_page;
	}
	else
	{
		t_meta_page *tmp = g_data.meta_pages;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = meta_page;
	}

	return meta_page;
}


typedef struct s_free_meta_space
{
	void *addr;
	t_meta_page *meta_page;
} t_free_meta_space;

// Iterate over all pages metadata and list already used bytes
// Do the same with blocks, then find out remaining free space
// Other option would be to make a t_metadata struct which contains an enum (E_PAGE and E_BLOCK), pointer to address and size (so t_page and t_block generic)
t_free_meta_space find_free_metadata_space(size_t size)
{
	t_free_meta_space free_meta_space = { .addr = NULL, .meta_page = NULL };

	for (t_meta_page *meta_page = g_data.meta_pages; meta_page != NULL; meta_page = meta_page->next)
	{
		// printf("meta page %p %p\n", meta_page->addr, g_data.blocks);
		char free_space[meta_page->size];
		for (size_t i = 0; i < meta_page->size; i++)
		{
			if (i < sizeof(t_meta_page))
				free_space[i] = 0; // first sizeof(t_meta_page) bytes are used to store the meta page metadata
			else
				free_space[i] = 1;
		}
		char *meta_page_start = (char *)meta_page->addr;
		char *meta_page_end = (char *)meta_page->addr + meta_page->size;

		for (t_page *page = g_data.pages; page != NULL; page = page->next)
		{
			// printf("page %p meta page %p\n", page->addr, meta_page->addr);
			char *page_addr = (char *)page;

			// We need to check every page even if page is not in meta_page, the blocks inside it might be
			// Just don't count space if page is not in meta_page
			if (page_addr >= meta_page_start && page_addr <= meta_page_end)
			{
				size_t offset = (char *)page - (char *)meta_page->addr;
				for (size_t i = 0; i < sizeof(t_page); i++)
					free_space[i + offset] = 0;
			}
		}

		// page can be stored on a meta_page and it's block on another, so check every single block and check if it's in meta_page
		for (t_block *block = g_data.blocks; block != NULL; block = block->next)
		{
			char *block_addr = (char *)block;
			// printf("Block %p is in meta page %p\n", block_addr, meta_page->addr);
			if (block_addr >= meta_page_start && block_addr <= meta_page_end)
			{
				// printf("inside\n");
				size_t offset = (char *)block - (char *)meta_page->addr;
				// block->addr points to the data page, block pointer itself is stored inside meta_page
				for (size_t i = 0; i < sizeof(t_block); i++)
					free_space[i + offset] = 0;
			}

		}

		// printf("[");
		// for (size_t i = 0; i < meta_page->size; i++)
		// {
		// 	printf("%c", free_space[i] + '0');
		// }
		// printf("]\n");

		size_t matched = 0;
		for (size_t i = 0; i < meta_page->size; i++)
		{
			if (free_space[i] == 1)
			{
				matched++;
				if (matched >= size)
				{
					free_meta_space.addr = (char *)meta_page->addr + i + 1 - matched; // + 1 since i starts at 0
					free_meta_space.meta_page = meta_page;
					return free_meta_space;
				}
			}
			else
				matched = 0;
		}
	}

	return free_meta_space;
}


// This functions simply finds free space for metadata (or mmap more memory if needed) and return an empty metadata t_page
t_page *create_page_metadata()
{
	printf("--- CREATE PAGE METADATA ---\n");
	t_free_meta_space free_space = find_free_metadata_space(sizeof(t_page));
	t_page *page = NULL;

	if (free_space.addr)
	{
		printf("There is free space for page at %p (meta page %p)\n", free_space.addr, free_space.meta_page->addr);
		t_meta_page *meta_page = free_space.meta_page;
		void *addr = free_space.addr;
		page = (t_page *)addr;

		ft_bzero(page, sizeof(t_page));

		if (!meta_page->first_page)
		{
			meta_page->first_page = page;
			meta_page->last_page = page;
		}
		else
		{
			t_page *tmp = meta_page->first_page;
			while (tmp->next)
				tmp = tmp->next;
			tmp->next = page;
		}
	}
	else
	{
		printf("There is no free space, allocating meta page\n");
		t_meta_page *meta_page = allocate_meta_page(1);
		// page is empty since we just created it
		void *addr = (char *)meta_page->addr + sizeof(t_meta_page);
		page = (t_page *)addr;
		printf("Allocated and initialized page at %p (meta page %p)\n", page, meta_page);

		ft_bzero(page, sizeof(t_page));

		meta_page->first_page = page;
		meta_page->last_page = page;

		// set page next from previous page (so from g_data.pages it's simpler)
		if (g_data.pages)
		{
			t_page *tmp = g_data.pages;
			while (tmp->next)
				tmp = tmp->next;
			tmp->next = page;
		}
	}


	if (!g_data.pages)
		g_data.pages = page;

	return page;
}

t_block *create_block_metadata(t_page *page)
{
	printf("--- CREATE BLOCK METADATA ---\n");
	t_free_meta_space free_space = find_free_metadata_space(sizeof(t_block));
	t_block *block = NULL;

	if (free_space.addr)
	{
		printf("There is free space for block at %p (meta page %p)\n", free_space.addr, free_space.meta_page->addr);
		block = (t_block *)free_space.addr;
	}
	else
	{
		printf("There is no free space, allocating meta page\n");
		t_meta_page *meta_page = allocate_meta_page(1);
		// page is empty since we just created it
		void *addr = (char *)meta_page->addr + sizeof(t_meta_page);
		block = (t_block *)addr;
		printf("Allocated and initialized block at %p (meta page %p)\n", block, meta_page);
	}

	ft_bzero(block, sizeof(t_block));

	if (!page->first)
	{
		page->first = block;
		page->last = block;
	}
	else
	{
		t_block *tmp = page->first;
		while (tmp != page->last)
			tmp = tmp->next;
		t_block *last_next = tmp->next;
		tmp->next = block;
		block->prev = tmp;
		block->next = last_next;
		page->last = block;
	}

	if (!g_data.blocks)
		g_data.blocks = block;

	return block;
}