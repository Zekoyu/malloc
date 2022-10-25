#ifndef FT_MALLOC_H
#define FT_MALLOC_H

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#define TINY_FT_MALLOC_MAX_SIZE 1024
#define SMALL_FT_MALLOC_MAX_SIZE 4096

typedef struct s_block
{
	void *addr;
	size_t real_size;
	struct s_block *next;
	struct s_block *prev; // keep prev in case of free so that we can easily remove it without traversing the whole chained list
} t_block;

typedef struct s_page
{
	void *addr;
	size_t real_size;
	struct s_page *next;
	struct s_page *prev;
	t_block *first;
	t_block *last;
} t_page;

typedef struct s_malloc_data
{
	t_page *tiny_pages;
	t_page *small_pages;
	t_page *large_pages;
} t_malloc_data;

extern t_malloc_data g_data;

typedef struct s_find_block_data
{
	t_block	*block;
	t_page *page;
} t_find_block_data;

t_find_block_data find_block_data(void *ptr, t_page *pages);
void internal_free(t_find_block_data data);
t_block *allocate_block(size_t size, t_page **pages);

void *ft_malloc(size_t size);
void *ft_realloc(void *ptr, size_t size);
void ft_free(void *ptr);
void show_alloc_mem();

#endif