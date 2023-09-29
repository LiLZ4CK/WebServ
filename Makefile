CC = c++ 

CFLAGS= -Wall -Wextra -Werror -std=c++98 #-g -fsanitize=address

SRCS =	main.cpp \
	  serverSide/client.cpp \
	  serverSide/server.cpp \
	  serverSide/socket.cpp \
	  serverSide/post.cpp \
	  serverSide/delete.cpp \
	  parsing/config.cpp \
	  parsing/request.cpp\
	  serverSide/cgi_handler.cpp

NAME = webserv

REM = rm -f

all : $(NAME)

%.o:%.cpp
	$(CC) $(CFLAGS) -c $(SRCS)

$(NAME): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(NAME)

clean :
	${REM} ${NAME}

fclean : clean
	${REM} ${NAME}

re : fclean all

.PHONY : all clean fclean re