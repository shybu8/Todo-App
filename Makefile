.PHONY: clean

NAME := todo-app
CC := g++
CFLAGS := -Wall -Wextra -Wpedantic -ggdb
SRC_CPP := main.cpp todo.cpp
SRC_HPP := todo.hpp

all: $(SRC_CPP) $(SRC_HPP)
	$(CC) $(SRC_CPP) -o $(NAME) $(CFLAGS)

clean:
	rm $(NAME)
