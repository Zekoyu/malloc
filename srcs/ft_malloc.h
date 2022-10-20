#ifndef FT_MALLOC_H
#define FT_MALLOC_H

#include <stdlib.h>

typedef struct s_block
{
	// size_t alignment_offset; // will see later if needed
	void *addr; // char * for arithmetic
	char *real_addr;
	size_t size; // malloced size
	size_t real_size; // real size (including this struct)
	struct s_block *next;
	struct s_block *prev; // keep prev in case of free so that we can easily remove it without traversing the whole chained list
} t_block;

typedef struct s_page
{
	// because first sizeof(t_page) bytes contains this struct, real data storage starts at (addr + sizeof(t_page))
	void *addr; // char * for arithmetic
	char *real_addr;
	t_block *first; // first block of page
	t_block *last; // last block of page
	size_t size;
	size_t real_size;
	struct s_page *next;
} t_page;

void *ft_malloc(size_t size);
void show_alloc_mem();

#endif