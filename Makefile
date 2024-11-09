NAME=webserv

CPPFLAGS= -Wall -Wextra -Werror -std=c++98

SRCS = $(wildcard */*.cpp) main.cpp

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