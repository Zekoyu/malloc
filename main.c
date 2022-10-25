#include "./srcs/ft_malloc.h"
// #include "./libft/libft.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define POUET_DEBUG 0

typedef struct s_vec3
{
	double	x;
	double	y;
	double	z;
}	t_vec3;

typedef struct s_test
{
	double	bitch;
	double	lasagna[50];
	t_vec3	random0;
	t_vec3	random1;
	t_vec3	random2;
	t_vec3	random3;
	t_vec3	random4;
}	t_tg;

int main()
{
	void *pouet;
	size_t malloc_total = 0;

	// pouet = ft_malloc(2060);
	// pouet = ft_malloc(3000);
	// pouet = ft_malloc(25);

	srand(time(NULL));

	for (int j = 0; j < 120; j++)
	{

		int r = rand() % 4;
		r = j % 4;

		void *pouic = NULL;

		if (r == 0)
			pouic = ft_malloc(1024);
		else if (r == 1)
			pouic = ft_malloc(2048);
		else if (r == 2)
			pouic = ft_malloc(12345);
		else
			pouic = NULL;

		void *test = ft_malloc(1024);
		malloc_total += 1024;

		((char *)test)[0] = '0';
		if (pouic)
			((char *)pouic)[100] = '1';
		// void *test2 = ft_malloc(2048);
		// void *test3 = ft_malloc(12345);

		// malloc_total += 1024 + 2048 + 12345;

		ft_free(pouic);
		// printf("-----------------\n");
		// printf("Freed address %p (r = %d)\n", pouic, r);
		// show_alloc_mem();
		// printf("Malloc'ed address is %p\n", test);
		// test = ft_realloc(test, 512);
		// printf("Realloc'ed smaller address is %p\n", test);
		// test = ft_realloc(test, 2048);
		// printf("Realloc'ed bigger address is %p\n", test);
		// ft_free(test);
		// t_tg	*oui;

		// oui = malloc(sizeof(t_tg) * 3);
		// ft_bzero(oui, sizeof(t_tg) * 3);

		// for (int i = 0; i < 3; i++)
		// {
		// 	t_tg	current = oui[i];
		// 	current.bitch = 1;
		// 	current.random0 = (t_vec3){0, 1, 2};
		// 	current.random1 = (t_vec3){0, 1, 2};
		// 	current.random2 = (t_vec3){0, 1, 2};
		// 	current.random3 = (t_vec3){0, 1, 2};
		// 	current.random4 = (t_vec3){0, 1, 2};
		// }
	}

	// size_t meta_mmap = get_mmap_meta_count();
	// size_t data_mmap = get_mmap_data_count();
	// printf("%zu metadata mmap and %zu data mmap\nTotal : %zu mmap\n", meta_mmap, data_mmap, meta_mmap + data_mmap);

	// show_alloc_pages();
	show_alloc_mem();
	printf("main malloc total is %zu\n", malloc_total);
	// show_struct_mem_space();
	// show_alloc_pages();
	return (0);
}