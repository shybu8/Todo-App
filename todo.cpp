#include "todo.hpp"
#include <cstdlib>
#include <iostream>
#include <string_view>

std::optional<Command> parse_command(char *arg) {
  std::string_view cmd(arg);
  for (unsigned i = 0; i < command_to_index(Command::CommandListLen); i++) {
    if (cmd == commandLiterals[i])
      return static_cast<Command>(i);
  }
  return std::nullopt;
}

std::optional<fs::path> todo_dir_path_opt() {
  const std::string_view homeRelativePath = ".todo";

  char *home = std::getenv("HOME");
  if (!home)
    return std::nullopt;
  fs::path todo_path(home);
  todo_path.append(homeRelativePath);
  return todo_path;
}

void list_todos(fs::path todo_dir_path) {
  try {
    unsigned entry_num = 1;
    for (auto &entry : fs::directory_iterator(todo_dir_path)) {
      if (entry.is_regular_file()) {
        std::cout << entry_num << ". " << entry.path().filename().string()
                  << '\n';
        entry_num++;
      }
    }
  } catch (const fs::filesystem_error &e) {
    std::cerr << "filesystem error: " << e.what() << '\n';
    return;
  }
}
