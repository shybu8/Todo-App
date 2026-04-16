#include "todo.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string_view>

using std::cerr;
using std::cin;
using std::cout;
using std::optional;
using std::string;
using std::string_view;

optional<Command> parse_command(const char *arg) {
  string_view cmd(arg);
  for (size_t i = 0; i < command_to_index(Command::CommandListLen); i++) {
    if (cmd == commandLiterals[i])
      return static_cast<Command>(i);
  }
  return std::nullopt;
}

optional<Status> parse_status(const char *arg) {
  string_view cmd(arg);
  for (size_t i = 0; i < status_to_index(Status::StatusListLen); i++) {
    if (cmd == statusLiterals[i])
      return static_cast<Status>(i);
  }
  return std::nullopt;
}

optional<fs::path> todo_dir_path_opt() {
  const string_view homeRelativePath = ".todo";

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

void list_todos(const fs::path &todo_dir_path) {
  try {
    size_t entry_num = 1;
    for (const auto &entry : fs::directory_iterator(todo_dir_path)) {
      if (entry.is_regular_file()) {
        std::ifstream file(entry.path());
        if (!file) {
          cerr << "ERROR: Unable to read file";
          continue;
        }
        string first_word, prefix;
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

void add_todo(const fs::path &todo_dir_path) {
  string name;
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
  std::ostringstream content_ss;
  content_ss << cin.rdbuf();

  const auto todo = Todo(std::move(name), content_ss.str(), Status::Undone);
  todo.save(todo_dir_path);
}

void get_todo(const fs::path &todo_dir_path, size_t num) {
  optional<fs::path> target_path(num_entry_path(todo_dir_path, num));
  if (!target_path.has_value()) {
    cout << "No such entry\n";
    return;
  }
  const auto todo = Todo(target_path.value());
  cout << todo.content;
}

void rm_todo(const fs::path &todo_dir_path, size_t num) {
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

void set_status(const fs::path &todo_dir_path, size_t num, Status sts) {
  const optional<fs::path> file_path(num_entry_path(todo_dir_path, num));
  if (!file_path) {
    cout << "No such entry\n";
    return;
  }
  auto todo = Todo(file_path.value());
  todo.status = sts;
  todo.save(todo_dir_path);
}

Todo::Todo(string name_, string content_, Status status_) noexcept
    : name(std::move(name_)), content(std::move(content_)), status(status_) {}

Todo::Todo(const fs::path &todo_file) : name(todo_file.filename().string()) {
  std::ifstream file(todo_file);
  if (!file)
    throw std::runtime_error("Unable to open file");

  string status_str;
  if (!std::getline(file, status_str)) {
    throw std::runtime_error("Unable to read todo status");
  }

  optional<Status> status = parse_status(status_str.c_str());
  if (!status) {
    cerr << "Unrecognized todo status, falling back to 'undone'\n";
    status = Status::Undone;
  }
  this->status = status.value();

  std::ostringstream content;
  content << file.rdbuf();
  this->content = content.str();
}

void Todo::save(const fs::path &todo_dir) const {
  const fs::path file_path = todo_dir / name;
  std::ofstream file(file_path, std::ios::trunc);
  if (!file)
    throw std::runtime_error("Unable to open file for writing");
  file << statusLiterals[status_to_index(status)] << '\n';
  file << content;
}
