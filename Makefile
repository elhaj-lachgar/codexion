CC = cc

CFLAGS = -Wall -Wextra -Werror -pthread

NAME = codexion

INCLUDE = -Iincludes

SRCS = ./src/main.c ./src/validator.c ./src/set_config.c ./src/init_config.c ./src/time.c ./src/routine.c \
		./src/free_handler.c ./src/heap.c ./src/watcher.c ./src/utils.c ./src/start_coders.c ./src/heap_utils.c \
		./src/add_utils.c ./src/init_config_utils.c ./src/routine_utils1.c ./src/routine_utils2.c

OBJS = $(SRCS:.c=.o)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o : %.c ./includes/codexion.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean :
	rm -rf $(OBJS)

fclean : clean
	rm -rf $(NAME)

re : fclean all
