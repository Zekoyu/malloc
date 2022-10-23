#include "./srcs/ft_malloc.h"
// #include "./libft/libft.h"
#include <unistd.h>
#include <stdio.h>

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

	// pouet = ft_malloc(2060);
	// pouet = ft_malloc(3000);
	// pouet = ft_malloc(25);

	printf("Start test\n");
	for (int j = 0; j < 1024; j++)
	{
		void *test = ft_malloc(1024);
		printf("Malloc'ed address is %p\n", test);
		test = ft_realloc(test, 512);
		printf("Realloc'ed smaller address is %p\n", test);
		test = ft_realloc(test, 2048);
		printf("Realloc'ed bigger address is %p\n", test);
		ft_free(test);
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
	// show_alloc_mem();
	// show_struct_mem_space();
	// show_alloc_pages();
	return (0);
}