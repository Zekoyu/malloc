#include "./ft_malloc.h"
#include <libft.h>


void ft_puthex(size_t num)
{
	if (num != 0)
		ft_puthex(num / 16);
	write(STDOUT_FILENO, &("0123456789abcdef"[num % 16]), 1);
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
	size_t total_allocs = 0;

	for (t_page *page = pages; page != NULL; page = page->next)
	{
		for (t_block *block = page->first; block != NULL; block = block->next)
		{
			total_bytes += block->real_size - sizeof(t_block);
			total_allocs++;

			// avoid malloc :)
			ft_putstr_fd("0x", STDOUT_FILENO);
			ft_puthex((size_t)block->addr);
			ft_putstr_fd(" - 0x", STDOUT_FILENO);
			ft_puthex((size_t)((char *)block->addr + block->real_size));
			ft_putstr_fd(" : ", STDOUT_FILENO);
			ft_putnbr_fd(block->real_size - sizeof(t_block), STDOUT_FILENO);
			ft_putstr_fd(" bytes\n", STDOUT_FILENO);

			if (block == page->last)
				break;
		}
	}

	return total_bytes;
}

void show_alloc_mem()
{
	size_t total_bytes = 0;

	pthread_mutex_lock(&g_mutex);

	void *tiny_addr = g_data.tiny_pages;
	void *small_addr = g_data.small_pages;
	void *large_addr = g_data.large_pages;

	ft_putstr_fd("TINY : ", STDOUT_FILENO);
	if (tiny_addr)
	{
		ft_putstr_fd("0x", STDOUT_FILENO);
		ft_puthex((size_t) tiny_addr);
	}
	else
	{
		ft_putstr_fd("(null)", STDOUT_FILENO);
	}
	ft_putstr_fd("\n", STDOUT_FILENO);
	total_bytes += print_alloc_mem_in_pages(g_data.tiny_pages);

	ft_putstr_fd("SMALL : ", STDOUT_FILENO);
	if (small_addr)
	{
		ft_putstr_fd("0x", STDOUT_FILENO);
		ft_puthex((size_t) small_addr);
	}
	else
	{
		ft_putstr_fd("(null)", STDOUT_FILENO);
	}
	ft_putstr_fd("\n", STDOUT_FILENO);
	total_bytes += print_alloc_mem_in_pages(g_data.small_pages);

	ft_putstr_fd("LARGE : ", STDOUT_FILENO);
	if (large_addr)
	{
		ft_putstr_fd("0x", STDOUT_FILENO);
		ft_puthex((size_t) large_addr);
	}
	else
	{
		ft_putstr_fd("(null)", STDOUT_FILENO);
	}
	ft_putstr_fd("\n", STDOUT_FILENO);
	total_bytes += print_alloc_mem_in_pages(g_data.large_pages);

	ft_putstr_fd("Total : ", STDOUT_FILENO);
	ft_putnbr_fd(total_bytes, STDOUT_FILENO);
	ft_putstr_fd(" bytes\n", STDOUT_FILENO);

	pthread_mutex_unlock(&g_mutex);
}