.PHONY: clean all

NAME := todo-app

CC := g++
CFLAGS := -Wall -Wextra -Wpedantic -ggdb -std=c++17

SRC_CPP := main.cpp todo.cpp utils.cpp todo_db.cpp
SRC_HPP := todo.hpp

all: $(NAME)

$(NAME): $(SRC_CPP) $(SRC_HPP)
	$(CC) $(SRC_CPP) -o $(NAME) $(CFLAGS)

clean:
	rm $(NAME)
