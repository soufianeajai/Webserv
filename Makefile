NAME=webserv

CPPFLAGS= -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp Connection.cpp HttpRequest.cpp HttpResponse.cpp HttpMessage.cpp Route.cpp Server.cpp WebServers.cpp 

CXX = c++

OBJ = $(SRSC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CPPFLAGS) $(OBJ) -o $(NAME)

fclean: clean
	rm -f $(NAME)

clean:
	rm -f $(OBJ)

re: fclean all

.PHONY: all clean fclean re