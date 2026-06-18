ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME    := libft_malloc_$(HOSTTYPE).so
LINK    := libft_malloc.so

CC      := cc
CFLAGS  := -Wall -Wextra -Werror -fPIC
CPPFLAGS  := -Iinclude -Iinternal
LDFLAGS := -shared

DBG_FLAGS = -g3 -O0 -fno-omit-frame-pointer

ifeq ($(DEBUG),1)
    CFLAGS += $(DBG_FLAGS)
else
    CFLAGS += -DNDEBUG -O2
endif


SRC_DIR   := src
OBJ_DIR   := obj

SRC       := \
	$(SRC_DIR)/malloc.c \
	$(SRC_DIR)/thread.c \
	$(SRC_DIR)/free.c \
	$(SRC_DIR)/realloc.c \
	$(SRC_DIR)/calloc.c \
	$(SRC_DIR)/heap.c \
	$(SRC_DIR)/block.c \
	$(SRC_DIR)/free_list.c \
	$(SRC_DIR)/coalesce.c \
	$(SRC_DIR)/mem_op.c \
	$(SRC_DIR)/show_mem.c

OBJ     := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
INC := includes/malloc.h internal/malloc_internal.h

all: $(NAME) 

$(NAME): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^
	ln -sf $(NAME) $(LINK)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c includes/malloc.h internal/malloc_internal.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(LINK)

re: fclean all


.PHONY: all clean fclean re 
