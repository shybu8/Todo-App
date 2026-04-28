.PHONY: all clean rebuild

CXX := g++
CXXFLAGS := -Wall -Wextra -Wpedantic -ggdb -std=c++17 -MMD -MP
LDLIBS :=

CLI_NAME := todo-app
SRV_NAME := todo-server

BUILD_DIR := build

COMMON_CPP := todo.cpp utils.cpp todo_db.cpp protocol.cpp

CLI_CPP := main.cpp $(COMMON_CPP)
SRV_CPP := server.cpp $(COMMON_CPP)

CLI_OBJ := $(CLI_CPP:%.cpp=$(BUILD_DIR)/cli/%.o)
SRV_OBJ := $(SRV_CPP:%.cpp=$(BUILD_DIR)/srv/%.o)

CLI_DEP := $(CLI_OBJ:.o=.d)
SRV_DEP := $(SRV_OBJ:.o=.d)

all: $(CLI_NAME) $(SRV_NAME)

$(CLI_NAME): $(CLI_OBJ)
	$(CXX) $^ -o $@ $(LDLIBS)

$(SRV_NAME): $(SRV_OBJ)
	$(CXX) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/cli/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/srv/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(CLI_DEP)
-include $(SRV_DEP)

clean:
	rm -rf $(BUILD_DIR) $(CLI_NAME) $(SRV_NAME)

rebuild: clean all
