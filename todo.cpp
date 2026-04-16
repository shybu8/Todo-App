#include "todo.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string_view>

using std::cerr;
using std::cin;
using std::cout;
using std::optional;

optional<Command> parse_command(char *arg) {
  std::string_view cmd(arg);
  for (size_t i = 0; i < command_to_index(Command::CommandListLen); i++) {
    if (cmd == commandLiterals[i])
      return static_cast<Command>(i);
  }
  return std::nullopt;
}

optional<Status> parse_status(char *arg) {
  std::string_view cmd(arg);
  for (size_t i = 0; i < status_to_index(Status::StatusListLen); i++) {
    if (cmd == statusLiterals[i])
      return static_cast<Status>(i);
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

optional<fs::path> num_entry_path(fs::path dir, size_t target_entry_num) {
  try {
    size_t entry_num = 1;
    for (const auto &entry : fs::directory_iterator(dir)) {
      if (entry.is_regular_file()) {
        if (entry_num == target_entry_num)
          return entry.path();
        entry_num++;
      }
    }

  } catch (const fs::filesystem_error &e) {
    cerr << "filesystem error: " << e.what() << '\n';
  }
  return std::nullopt;
}

void list_todos(fs::path todo_dir_path) {
  try {
    size_t entry_num = 1;
    for (const auto &entry : fs::directory_iterator(todo_dir_path)) {
      if (entry.is_regular_file()) {
        std::ifstream file(entry.path(), std::ios::in);
        if (!file) {
          cerr << "ERROR: Unable to read file";
          continue;
        }
        std::string first_word, prefix;
        file >> first_word;
        file.close();
        if (first_word == statusLiterals[status_to_index(Status::InProgress)])
          prefix = "> ";
        else if (first_word == statusLiterals[status_to_index(Status::Done)])
          prefix = "V ";
        cout << entry_num << ". " << prefix << entry.path().filename().string()
             << '\n';
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
  std::ofstream file(file_path, std::ios::out);
  if (!file) {
    cerr << "ERROR: Unable to open a file\n";
    return;
  }
  file << "UNDONE\n";
  file << cin.rdbuf();
}

void get_todo(fs::path todo_dir_path, size_t num) {
  optional<fs::path> target_path(num_entry_path(todo_dir_path, num));
  if (!target_path.has_value()) {
    cout << "No such entry exists\n";
    return;
  }
  std::ifstream file(target_path.value(), std::ios::in);
  if (!file) {
    cerr << "ERROR: Unable to open a file\n";
    return;
  }
  file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  cout << file.rdbuf();
}

void rm_todo(fs::path todo_dir_path, size_t num) {
  optional<fs::path> file_path(num_entry_path(todo_dir_path, num));
  if (!file_path.has_value()) {
    cout << "No such entry\n";
    return;
  }
  cout << "Do you really want to delete " << file_path.value().filename()
       << "? [Y/n]: ";
  char ans(std::getchar());
  if (ans == 'y' || ans == '\n') {
    std::error_code ec;
    fs::remove(file_path.value(), ec);
    if (ec) {
      cerr << "ERROR: Unable to remove file: " << ec.message() << '\n';
      return;
    }
  } else
    cout << "Cancelled\n";
}

void set_status(fs::path todo_dir_path, size_t num, Status sts) {
  const optional<fs::path> file_path(num_entry_path(todo_dir_path, num));
  if (!file_path.has_value()) {
    cout << "No such entry\n";
    return;
  }
  std::ifstream file_read(file_path.value(), std::ios::in);
  if (!file_read) {
    cerr << "ERROR: Unable to open file for reading\n";
    return;
  }
  file_read.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  std::ostringstream content;
  content << file_read.rdbuf();
  file_read.close();

  std::ofstream file_write(file_path.value(), std::ios::trunc);
  if (!file_write) {
    cerr << "ERROR: Unable to open file for writing (trunc)\n";
    return;
  }
  file_write << statusLiterals[status_to_index(sts)] << '\n';
  file_write << content.str();
  file_write.close();
}
