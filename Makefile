SRCS = $(wildcard srcs/*.c)

OBJS = $(SRCS:.c=.o)

LIBFT = ./libft/libft.a
LIBFT_FOLDER = ./libft

ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME := libft_malloc_$(HOSTTYPE).so

CC = gcc

CFLAGS = -I$(LIBFT_FOLDER) -lpthread -g -fsanitize=address # -Wall -Wextra -Werror
DEBUG_CFLAGS = -I$(LIBFT_FOLDER) -lpthread -g -fsanitize=address -DFT_MALLOC_BACK_GUARD=32 -DFT_MALLOC_FRONT_GUARD=32 -DFT_MALLOC_DEBUG=1

OS := $(shell uname)

SHELL = zsh

AQUA = \033[0;96m
AQUA_BOLD = \033[1;96m

PURPLE = \033[0;95m
PURPLE_BOLD = \033[1;95m

GREEN = \033[0;92m
GREEN_BOLD = \033[1;92m
GREEN_UNDRLINE = \033[4;32m

RED = \033[0;91m
IRED = \033[0;31m
RED_BOLD = \033[1;91m

SAME_LINE = \033[0G\033[2K

RESET = \033[0m

%.o: %.c
ifdef DEBUG
	@$(CC) $(DEBUG_CFLAGS) -c $< -o $@
else
	@$(CC) $(CFLAGS) -c $< -o $@
endif
	@echo -n "$(SAME_LINE)$(AQUA)Compiling $(AQUA_BOLD)$<$(RESET)"

$(NAME):	$(LIBFT) $(OBJS)
	@echo
	@echo "$(PURPLE)Linking ($(PURPLE)*.o) into $(PURPLE_BOLD)$(NAME)$(RESET)"
	@ar rcs $(NAME) $(OBJS)
# https://stackoverflow.com/questions/3821916/how-to-merge-two-ar-static-libraries-into-one
# merge libmalloc and libft into libmalloc
	@echo "$(PURPLE)Merging ($(PURPLE)$(NAME) $(LIBFT)) into $(PURPLE_BOLD)$(NAME)$(RESET)"

	@if [ $(OS) = "Darwin" ]; then \
	libtool -static -o $(NAME) $(NAME) $(LIBFT) ;\
	fi

	@if [ $(OS) = "Linux" ]; then \
	mkdir tmp ;\
	ar x --output tmp $(LIBFT) ;\
	ar x --output tmp $(NAME) ;\
	ar rcs $(NAME) tmp/*.o ;\
	rm -rf tmp ;\
	fi

	@echo "$(AQUA)Creating symlink ($(AQUA_BOLD)libft_malloc.so $(AQUA)-> $(AQUA_BOLD)$(NAME)$(AQUA))$(RESET)"
	@ln -s -f ./$(NAME) ./libft_malloc.so
	@echo "$(GREEN_BOLD)Done compiling $(GREEN_UNDERLINE)$(NAME)$(RESET)"

all:		$(NAME)

test: $(NAME) main.o
	@echo
ifdef DEBUG
	@$(CC) $(DEBUG_CFLAGS) main.o -lft_malloc -L./ -o test
else
	@$(CC) $(CFLAGS) main.o -lft_malloc -L./ -o test
endif
	@echo "$(GREEN_BOLD)Done compiling $(GREEN_UNDERLINE)test$(RESET)"

debug: fclean
	@$(MAKE) test DEBUG=1

$(LIBFT):
	@$(MAKE) -C $(LIBFT_FOLDER) NO_FT_MALLOC=1 NO_GNL=1

clean:
	@rm -f $(OBJS)
	@echo "$(RED)Removing $(IRED)*.o$(RESET)"
	@make -C $(LIBFT_FOLDER) clean

fclean:		clean
	@rm -f $(NAME) libft_malloc.so test
	@echo "$(RED)Removing $(IRED)$(NAME) $(RED), $(IRED)libft_malloc.so $(RED)and $(IRED)test$(RESET)"
	@make -C $(LIBFT_FOLDER) fclean

re:			fclean all

.PHONY:	all clean fclean re