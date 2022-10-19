#ifndef FT_MALLOC_H
#define FT_MALLOC_H

#include <stdlib.h>

typedef struct s_block
{
	// size_t alignment_offset; // will see later if needed
	void *addr;
	size_t size;
	struct s_block *next;
	struct s_block *prev; // keep prev in case of free so that we can easily remove it without traversing the whole chained list
} t_block;

typedef struct s_page
{
	// because first sizeof(t_page) bytes contains this struct, real data storage starts at (addr + sizeof(t_page))
	void *addr;
	t_block *alloc; // address of the first allocated block (NULL if none)
	t_block *free; // address of the first free address in the page (NULL if none)
	size_t size;
	struct s_page *next;
} t_page;

void *malloc(size_t size);

#endif