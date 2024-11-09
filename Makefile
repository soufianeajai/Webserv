NAME=webserv

CPPFLAGS= -Wall -Wextra -Werror -std=c++98 -g

SRCS = main.cpp ParsingConfig/ParsingConfig.cpp Server/Server.cpp Route/Route.cpp WebServers/Webservers.cpp serverSetup/ServerSetup.cpp # Connection.cpp HttpRequest.cpp HttpResponse.cpp HttpMessage.cpp WebServers.cpp 

CXX = c++

OBJ = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CPPFLAGS) $(OBJ) -o $(NAME)

fclean: clean
	rm -f $(NAME)

clean:
	rm -f $(OBJ)

re: fclean all

.PHONY: all clean fclean re