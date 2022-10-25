#include <libft.h>
#include "./ft_malloc.h"

/**
 * @brief Creates a new page with the specified size, sets it's metadata and returns it.
 *
 * @param size the size of the page to create (will be rounded up to the nearest multiple of getpagesize())
 * @param pages the page list in which to store the new page
 * @return t_page*
 */
t_page *add_new_page(size_t size, t_page **pages)
{
	void *addr;

	size_t page_size = getpagesize();
	size_t alloc_size = size / page_size * page_size + (size % page_size == 0 ? 0 : page_size);

	if ((addr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED) // we dont care about last 2 args(fd and offset)
		return NULL;

	t_page *page = (t_page *)addr;
	ft_bzero(page, sizeof(t_page));
	page->addr = addr;
	page->real_size = alloc_size;

	if (!*pages)
		*pages = page;
	else
	{
		t_page *tmp = *pages;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = page;
		page->prev = tmp;
	}

	return page;
}

/**
 * @brief Find free space in the given pages and create a block if found(until page == NULL)
 *
 * @param size size of free space needed
 * @param pages the pages in which to search (linked list)
 * @return t_block * the newly created block
 * @return NULL if no free space found
 */
t_block *create_block_if_space(size_t size, t_page *pages)
{
	// real-block size is memory aligned on 8 bytes (see https://en.wikipedia.org/wiki/Data_structure_alignment)
	size_t real_block_size = size + sizeof(t_block);

	for (t_page *page = pages; page != NULL; page = page->next)
	{
		if (!page->first)
		{
			// page is always aligned so no need to check
			if (page->real_size - sizeof(t_page) < real_block_size)
				continue;

			// create block at the start of page (first sizeof(t_page) bytes are reserved for page metadata)
			t_block *block = (t_block *)((char *)page->addr + sizeof(t_page));
			ft_bzero(block, sizeof(t_block));
			block->addr = (void *)((char *)block + sizeof(t_block));
			block->real_size = real_block_size;
			page->first = block;

			for (t_page *tmp = page->prev; tmp != NULL; tmp = tmp->prev)
			{
				if (tmp->last)
				{
					tmp->last->next = block;
					block->prev = tmp->last;
					break;
				}
			}

			if (g_data.blocks == NULL)
				g_data.blocks = block;

			return block;
		}

		char *page_start_usable_addr = (char *)page->addr + sizeof(t_page);

		size_t free_before_first_block = (char *)page->first->addr - page_start_usable_addr;
		if (free_before_first_block >= real_block_size)
		{
			// sizeof(something) is always aligned so no need to check
			t_block *block = (t_block *)page_start_usable_addr;
			ft_bzero(block, sizeof(t_block));
			block->addr = (void *)((char *)block + sizeof(t_block));
			block->real_size = real_block_size;
			block->next = page->first;
			if (page->first)
				page->first->prev = block;
			page->first = block;

			for (t_page *tmp = page->prev; tmp != NULL; tmp = tmp->prev)
			{
				if (tmp->last)
				{
					tmp->last->next = block;
					block->prev = tmp->last;
					break;
				}
			}

			return block;
		}

		for (t_block *block = page->first; block->next != NULL && block != page->last; block = block->next)
		{
			// alignment_offset is the number of  bytes we should add to the address to make it aligned

			size_t alignment_offset = block->real_size % 8 == 0 ? 0 : 8 - block->real_size % 8;
			size_t free_between_blocks = (char *)block->next - ((char *)block + block->real_size + alignment_offset);

			if (free_between_blocks >= real_block_size)
			{
				t_block *new_block = (t_block *)((char *)block + block->real_size + alignment_offset);
				ft_bzero(new_block, sizeof(t_block));
				new_block->addr = (void *)((char *)new_block + sizeof(t_block));
				new_block->real_size = real_block_size;
				new_block->next = block->next;
				new_block->prev = block;
				block->next->prev = new_block;
				block->next = new_block;
				return new_block;
			}
		}

		size_t alignment_offset = page->last->real_size % 8 == 0 ? 0 : 8 - page->last->real_size % 8;
		size_t free_after_last_block = (char *)page->addr + page->real_size - ((char *)page->last + page->last->real_size + alignment_offset);
		if (free_after_last_block >= real_block_size)
		{
			t_block *block = (t_block *)((char *)page->last + page->last->real_size + alignment_offset);
			ft_bzero(block, sizeof(t_block));
			block->addr = (void *)((char *)block + sizeof(t_block));
			block->real_size = real_block_size;
			block->prev = page->last;
			page->last->next = block;
			page->last = block;
			return block;
		}
	}

	return NULL;
}

/**
 * @brief Either creates a new page or finds a page with enough free space and creates a block in it.
 *
 * @param size size of the block to create
 * @param pages the pages in which to search for free space and add the block
 * @return t_block *the addres of the newly created block
 */
t_block *allocate_block(size_t size, t_page **pages)
{
	t_block *new_block;

	new_block = create_block_if_space(size, *pages);

	if (new_block)
		return new_block;

	int page_size = getpagesize();
	t_page *new_page = NULL;

	// no free space found, allocate new page
	if (pages == &g_data.tiny_pages)
		new_page = add_new_page(TINY_FT_MALLOC_MAX_SIZE * 100, pages);
	else if (pages == &g_data.small_pages)
		new_page = add_new_page(SMALL_FT_MALLOC_MAX_SIZE * 100, pages);
	else if (pages == &g_data.large_pages)
		new_page = add_new_page(size, pages);

	if (!new_page)
	{
		printf("Cannot add new page (mmap failed)\n");
		return NULL;
	}

	new_block = (t_block *)((char *)new_page->addr + sizeof(t_page));
	ft_bzero(new_block, sizeof(t_block));
	new_block->addr = (void *)((char *)new_block + sizeof(t_block));
	new_block->real_size = size + sizeof(t_block);
	new_page->first = new_block;
	new_page->last = new_block;

	// Set previous block->next to block (check all pages until we find one page not empty)
	for (t_page *tmp = new_page->prev; tmp != NULL; tmp = tmp->prev)
	{
		if (tmp->last)
		{
			tmp->last->next = new_block;
			new_block->prev = tmp->last;
			break;
		}
	}

	if (g_data.blocks == NULL)
		g_data.blocks = new_block;

	return new_block;
}

/**
 * @brief Prints the allocated memory addresses and sizes in the given page list
 *
 * @param pages the pages in which to search for allocated blocks
 * @return size_t the sum of all memory blocks in the given pages
 */
size_t print_alloc_mem_in_pages(t_page *pages)
{
	size_t total_bytes = 0;

	for (t_page *page = pages; page != NULL; page = page->next)
	{
		for (t_block *block = page->first; block != NULL; block = block->next)
		{
			total_bytes += block->real_size - sizeof(t_block);

			printf("%p - %p : %zu bytes\n", block->addr, (char *)block->addr + block->real_size - sizeof(t_block), block->real_size - sizeof(t_block));
			if (block == page->last)
				break;
		}
	}

	return total_bytes;
}

void show_alloc_mem()
{
	size_t total_bytes = 0;

	void *tiny_addr = g_data.tiny_pages;
	void *small_addr = g_data.small_pages;
	void *large_addr = g_data.large_pages;

	printf("TINY : %p\n", tiny_addr);
	total_bytes += print_alloc_mem_in_pages(g_data.tiny_pages);

	printf("SMALL : %p\n", small_addr);
	total_bytes += print_alloc_mem_in_pages(g_data.small_pages);

	printf("LARGE : %p\n", large_addr);
	total_bytes += print_alloc_mem_in_pages(g_data.large_pages);

	printf("Total : %zu bytes\n", total_bytes);
}