#include "./ft_malloc.h"
#include <libft.h>

t_malloc_data g_data = { 0 };

size_t g_mmap_meta_count = 0;

t_meta_page *allocate_meta_page(size_t count)
{
	void *addr;
	size_t alloc_size = count * getpagesize();
	// allow read/write, private (only this process can see it), anonymous means it's not backed by a file, just for data storage
	if ((addr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED) // we dont care about last 2 args(fd and offset)
		return NULL;

	g_mmap_meta_count += count;

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

size_t get_mmap_meta_count()
{
	return g_mmap_meta_count;
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
		char free_space[meta_page->size];
		ft_memset(free_space, 1, meta_page->size);
		// first sizeof(t_meta_page) bytes are used to store the meta page metadata
		ft_memset(free_space, 0, sizeof(t_meta_page));

		char *meta_page_start = (char *)meta_page->addr;
		char *meta_page_end = (char *)meta_page->addr + meta_page->size;

		for (t_page *page = g_data.pages; page != NULL; page = page->next)
		{
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

		t_block *pouet = g_data.blocks;

		// page can be stored on a meta_page and it's block on another, so check every single block and check if it's in meta_page
		for (t_block *block = g_data.blocks; block != NULL; block = block->next)
		{
			char *block_addr = (char *)block;
			if (block_addr >= meta_page_start && block_addr <= meta_page_end)
			{
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


/**
 * @brief Create a page metadata object and store it in the metadata pages
 * Also manages initialization (setting page next if needed).
 * Also adds the page to the global struct g_data if needed, the caller should not modify it
 *
 * @return The address of the created metadata page or NULL on failure
 */
t_page *create_page_metadata()
{
	#if POUET_DEBUG
	printf("--- CREATE PAGE METADATA ---\n");
	#endif

	t_free_meta_space free_space = find_free_metadata_space(sizeof(t_page));
	t_page *page = NULL;

	if (free_space.addr)
	{
		#if POUET_DEBUG
		printf("There is free space for page at %p (meta page %p)\n", free_space.addr, free_space.meta_page->addr);
		#endif

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
			meta_page->last_page = page;
		}

		if (g_data.pages)
		{
			t_page *tmp = g_data.pages;
			while (tmp->next)
				tmp = tmp->next;
			// in case the page is not in the same meta_page as the previous page
			if (tmp != page && tmp->next != page)
				tmp->next = page;
		}
	}
	else
	{
		#if POUET_DEBUG
		printf("There is no free space, allocating meta page\n");
		#endif

		t_meta_page *meta_page = allocate_meta_page(1);
		if (!meta_page)
		{
			printf("Failed to allocate meta page\n");
			return NULL;
		}
		// page is empty since we just created it
		void *addr = (char *)meta_page->addr + sizeof(t_meta_page);
		page = (t_page *)addr;

		#if POUET_DEBUG
		printf("Allocated and initialized page at %p (meta page %p)\n", page, meta_page);
		#endif

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

t_block *get_last_metadata_block()
{
	t_block *last_block = NULL;

	for (t_page *page = g_data.pages; page != NULL; page = page->next)
	{
		for (t_block *block = page->first; block != NULL; block = block->next)
		{
			last_block = block;
			if (block == page->last)
				break;
		}
	}

	#if POUET_DEBUG
	printf("Last metadata block is at address %p\n", last_block ? last_block->addr : NULL);
	#endif

	return last_block;
}

/**
 * @brief Create a block metadata object and store it in the metadata pages.
 * Also manages initialization (setting page first/last if needed and block prev/next)
 * Also adds the block to the global struct g_data if needed, the caller should not modify it
 *
 * @param page the page in which the block is located
 * @return The address of the created metadata block or NULL on failure
 */
t_block *create_block_metadata(t_page *page)
{
	#if POUET_DEBUG
	printf("--- CREATE BLOCK METADATA in page %p ---\n", page->addr);
	#endif

	t_free_meta_space free_space = find_free_metadata_space(sizeof(t_block));
	t_block *block = NULL;

	if (free_space.addr)
	{
		#if POUET_DEBUG
		printf("There is free space for block at %p (meta page %p)\n", free_space.addr, free_space.meta_page->addr);
		#endif

		block = (t_block *)free_space.addr;
	}
	else
	{
		#if POUET_DEBUG
		printf("There is no free space, allocating meta page\n");
		#endif

		t_meta_page *meta_page = allocate_meta_page(1);
		if (!meta_page)
		{
			printf("Failed to allocate meta page\n");
			return NULL;
		}
		// page is empty since we just created it
		void *addr = (char *)meta_page->addr + sizeof(t_meta_page);
		block = (t_block *)addr;

		#if POUET_DEBUG
		printf("Allocated and initialized block at %p (meta page %p)\n", block, meta_page);
		#endif
	}

	ft_bzero(block, sizeof(t_block));

	if (!page->first)
	{
		#if POUET_DEBUG
		printf("Page %p is empty, setting block %p as first and last\n", page->addr, block->addr);
		#endif

		t_block *prev_last = NULL;

		for (t_page *tmp = g_data.pages; tmp != NULL; tmp = tmp->next)
		{
			// get last of previous page
			if (tmp->next == page)
			{
				prev_last = tmp->last; // should always be one because otherwise we would have found free space on this previous page
				break;
			}
		}

		page->first = block;
		page->last = block;

		if (prev_last)
			prev_last->next = block;
		block->prev = prev_last;
	}
	else
	{
		#if POUET_DEBUG
		printf("Page %p already has a first block, it's %p\n", page->addr, page->first->addr);
		#endif

		t_block *tmp = page->first;
		while (tmp != page->last)
			tmp = tmp->next;
		t_block *last_next = tmp->next;
		tmp->next = block;
		block->prev = tmp;
		block->next = last_next;
		if (block->next)
			block->next->prev = block;
		page->last = block;
	}

	if (!g_data.blocks)
		g_data.blocks = block;

	return block;
}


// returns total bytes of type
size_t show_alloc_mem_of_type(enum e_page_type type)
{
	size_t total = 0;

	for (t_page *page = g_data.pages; page != NULL; page = page->next)
	{
		if (page->type != type)
			continue;

		for (t_block *block = page->first; block != NULL; block = block->next)
		{
			total += block->real_size;
			printf("%p - %p : %zu bytes\n", block->addr, (char *)block->addr + block->real_size, block->real_size);

			if (block == page->last)
				break;
		}
	}

	return total;
}

void *get_first_addr_of_page_type(enum e_page_type type)
{
	for (t_page *page = g_data.pages; page != NULL; page = page->next)
	{
		if (page->type != type)
			continue;

		return page->addr;
	}

	return NULL;
}

void show_alloc_mem()
{
	size_t total_bytes = 0;

	void *tiny_addr = get_first_addr_of_page_type(E_PAGE_TYPE_TINY);
	void *small_addr = get_first_addr_of_page_type(E_PAGE_TYPE_SMALL);
	void *large_addr = get_first_addr_of_page_type(E_PAGE_TYPE_LARGE);

	if (tiny_addr)
	{
		printf("TINY : %p\n", tiny_addr);
		total_bytes += show_alloc_mem_of_type(E_PAGE_TYPE_TINY);
	}
	else
	{
		printf("TINY : None\n");
	}

	if (small_addr)
	{
		printf("SMALL : %p\n", small_addr);
		total_bytes += show_alloc_mem_of_type(E_PAGE_TYPE_SMALL);
	}
	else
	{
		printf("SMALL : None\n");
	}

	if (large_addr)
	{
		printf("LARGE : %p\n", large_addr);
		total_bytes += show_alloc_mem_of_type(E_PAGE_TYPE_LARGE);
	}
	else
	{
		printf("LARGE : None\n");
	}

	printf("Total : %zu bytes\n", total_bytes);
}


void show_alloc_pages()
{
	for (t_page *page = g_data.pages; page != NULL; page = page->next)
	{
		printf("Page %p (size %zu), first block: %p, last block: %p\n", page->addr, page->real_size, page->first ? page->first->addr : NULL, page->last ? page->last->addr : NULL);
	}
}
