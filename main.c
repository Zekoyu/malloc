#include "./srcs/ft_malloc.h"

int main()
{
	show_alloc_mem();
	void *test = ft_malloc(123);
	show_alloc_mem();
}