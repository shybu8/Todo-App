.PHONY: clean all

CLI_NAME := todo-app
SRV_NAME := todo-server

CC := g++
CFLAGS := -Wall -Wextra -Wpedantic -ggdb -std=c++17

SRC_CLI_CPP := main.cpp todo.cpp utils.cpp todo_db.cpp protocol.cpp
SRC_CLI_HPP := todo.hpp utils.hpp todo_db.hpp enums_literals.hpp protocol.hpp

SRC_SRV_CPP := server.cpp todo_db.cpp utils.cpp todo.cpp protocol.cpp
SRC_SRV_HPP := todo_db.hpp utils.hpp todo.hpp protocol.hpp visitor_pattern.hpp

all: $(CLI_NAME) $(SRV_NAME)

$(CLI_NAME): $(SRC_CLI_CPP) $(SRC_CLI_HPP)
	$(CC) $(SRC_CLI_CPP) -o $(CLI_NAME) $(CFLAGS)

$(SRV_NAME): $(SRC_SRV_CPP) $(SRC_SRV_HPP)
	$(CC) $(SRC_SRV_CPP) -o $(SRV_NAME) $(CFLAGS)

clean:
	rm $(CLI_NAME)
