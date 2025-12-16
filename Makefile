CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98
NAME = ircserv
SRC = src/main.cpp\
	src/Server.cpp\
	src/Client.cpp\
	src/Command/*.cpp
OBJ = $(SRC:.cpp=.o)
INCLUDES = -I ./includes

all:$(NAME)

$(NAME):$(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

./PHONY: all clean fclean re