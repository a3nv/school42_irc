CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98
CPPFLAGS = -I ./includes
NAME = ircserv
SRC = src/main.cpp\
	src/Server.cpp\
	src/Client.cpp\
	src/Signal.cpp

OBJ = $(SRC:.cpp=.o)
INCLUDES = -I ./includes

all:$(NAME)

$(NAME):$(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
