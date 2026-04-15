#include "todo.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

using std::cerr;
using std::cin;
using std::cout;
using std::optional;

optional<Command> parse_command(char *arg) {
  std::string_view cmd(arg);
  for (unsigned i = 0; i < command_to_index(Command::CommandListLen); i++) {
    if (cmd == commandLiterals[i])
      return static_cast<Command>(i);
  }
  return std::nullopt;
}

optional<fs::path> todo_dir_path_opt() {
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
    for (const auto &entry : fs::directory_iterator(todo_dir_path)) {
      if (entry.is_regular_file()) {
        cout << entry_num << ". " << entry.path().filename().string() << '\n';
        entry_num++;
      }
    }
  } catch (const fs::filesystem_error &e) {
    cerr << "filesystem error: " << e.what() << '\n';
    return;
  }
}

void add_todo(fs::path todo_dir_path) {
  std::string name;
  fs::path file_path;
  while (true) {
    cout << "Name: ";
    if (!std::getline(cin, name))
      return;
    if (name.empty()) {
      cout << "Name can not be empty\n";
      continue;
    }
    file_path = todo_dir_path / name;
    if (fs::exists(file_path)) {
      cout << "This name already exists, try another\n";
      continue;
    }
    break;
  }
  cout << "Content (Ctrl+D to done):\n";
  std::fstream file(file_path, std::ios::out);
  if (!file) {
    cerr << "ERROR: Unable to open a file\n";
    return;
  }
  file << cin.rdbuf();
}

void get_todo(fs::path todo_dir_path, size_t num) {
  optional<fs::path> target_path;
  try {
    size_t i = 1;
    for (const auto &entry : fs::directory_iterator(todo_dir_path)) {
      if (entry.is_regular_file()) {
        if (i == num)
          target_path = entry.path();
        i++;
      }
    }
  } catch (const fs::filesystem_error &e) {
    cerr << "filesystem error: " << e.what() << '\n';
  }
  if (!target_path.has_value()) {
    cout << "No such entry exists\n";
    return;
  }
  std::fstream file(target_path.value(), std::ios::in);
  if (!file) {
    cerr << "ERROR: Unable to open a file\n";
    return;
  }
  cout << file.rdbuf();
}
