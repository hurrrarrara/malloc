ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME    := libft_malloc_$(HOSTTYPE).so
LINK    := libft_malloc.so

CC      := cc
CFLAGS  := -Wall -Wextra -Werror -fPIC -g3 -Iinternal
LDFLAGS := -shared

SRC     := includes/malloc.c
OBJ     := $(SRC:.c=.o)

all: $(NAME) symlink

$(NAME): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c includes/malloc.h
	$(CC) $(CFLAGS) -c $< -o $@

symlink: $(NAME)
	ln -sf $(NAME) $(LINK)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME) $(LINK)

re: fclean all

.PHONY: all clean fclean re symlink
