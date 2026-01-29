CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98
CPPFLAGS = -I ./includes
NAME = ircserv
SRC = src/main.cpp\
	src/Channel.cpp\
	src/Client.cpp\
	src/Server.cpp\
	src/Signal.cpp\
	src/Command/Cap.cpp\
	src/Command/Command.cpp\
	src/Command/Invite.cpp\
	src/Command/Join.cpp\
	src/Command/Kick.cpp\
	src/Command/Mode.cpp\
	src/Command/Names.cpp\
	src/Command/Nick.cpp\
	src/Command/Notice.cpp\
	src/Command/Part.cpp\
	src/Command/Pass.cpp\
	src/Command/Ping.cpp\
	src/Command/Pong.cpp\
	src/Command/PrivMsg.cpp\
	src/Command/Quit.cpp\
	src/Command/Topic.cpp\
	src/Command/User.cpp\


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
