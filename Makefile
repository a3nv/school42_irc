CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98
CPPFLAGS = -I ./includes
NAME = ircserv
SRC = src/main.cpp\
	src/Server.cpp\
	src/Client.cpp\
	src/Signal.cpp\
	src/Channel.cpp\
	src/Command/Command.cpp\
	src/Command/Nick.cpp\
	src/Command/Notice.cpp\
	src/Command/Pass.cpp\
	src/Command/Ping.cpp\
	src/Command/Pong.cpp\
	src/Command/PrivMsg.cpp\
	src/Command/Quit.cpp\
	src/Command/User.cpp\
	src/Command/Cap.cpp\
	src/Command/Join.cpp\
	src/Command/Part.cpp\
	src/Command/Names.cpp\


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
