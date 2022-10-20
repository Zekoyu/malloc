#include "./srcs/ft_malloc.h"

int main()
{
	ft_malloc(2000);
	ft_malloc(12);
	ft_malloc(1);
	ft_malloc(2);
	ft_malloc(80000);
	ft_malloc(2);
	ft_malloc(2000);
	show_alloc_mem();
}