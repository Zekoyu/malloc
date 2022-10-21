#ifndef FT_MALLOC_H
#define FT_MALLOC_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct s_block
{
	void *addr;
	size_t size;
	struct s_block *next;
	struct s_block *prev; // keep prev in case of free so that we can easily remove it without traversing the whole chained list
} t_block;

typedef struct s_page
{
	void *addr;
	t_block *first; // first block of page
	t_block *last; // last block of page
	size_t size;
	struct s_page *next;
} t_page;

typedef struct s_meta_page // used to store malloced pages/blocks
{
	void *addr; // address of page (addr[0] = this struct)
	size_t size; // size of page (multiple of getpagesize())
	struct s_meta_page *next; // next local page
	t_page *first_page; // first page in this metadata page (so we can iterate through)
	t_page *last_page;
} t_meta_page;

/*
	Memory layout
                                               +------------------+---------+
                                               |                  V         V
	+--------------------------------------+--------+--------+---------+---------+
	| META_PAGE  -  Metadata (t_meta_page) | t_page | t_page | t_block | t_block |
	+--------------------------------------+--------+--------+---------+---------+
	        +---------------------------------+                   |         |
            |              +--------------------------------------+         |
            |              |              +---------------------------------+
            V              V              V
	+-------------+---------------+---------------+
	| (DATA) PAGE | DATA (void *) | DATA (void *) |
	+-------------+---------------+---------------+

	Local page stores all metadata, and "standard" page only stores data
*/

typedef struct s_malloc_data
{
	t_page *pages;
	t_block *blocks;
	t_meta_page *meta_pages;
} t_malloc_data;


extern t_malloc_data g_data;

t_page *create_page_metadata();
t_block *create_block_metadata(t_page *page);
size_t get_mmap_meta_count();
size_t get_mmap_data_count();
t_page *allocate_page(size_t count);
void *find_or_alloc_space(size_t size);


void *ft_malloc(size_t size);
void show_alloc_mem();
void show_struct_mem_space();
void show_alloc_pages();


#endif