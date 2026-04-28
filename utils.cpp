#include "utils.hpp"
#include "todo.hpp"
#include <asio.hpp>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

namespace fs = std::filesystem;
using std::cerr;
using std::cin;
using std::cout;
using std::optional;
using std::string;
using std::string_view;
using std::unique_ptr;

optional<fs::path> default_todo_dir_path_opt() {
  const string_view homeRelativePath = ".todo";

  char *home = std::getenv("HOME");
  if (!home)
    return std::nullopt;
  fs::path todo_path(home);
  todo_path.append(homeRelativePath);
  return todo_path;
}

optional<unique_ptr<TodoDB>> determine_backend(asio::io_context &io,
                                               char *todo_data) {
  unique_ptr<TodoDB> todo_db;
  if (todo_data == nullptr) {
    // Getting default dir path
    optional<fs::path> todo_dir_path = default_todo_dir_path_opt();
    if (!todo_dir_path.has_value()) {
      cerr << "ERROR: Unable to get homedir path";
      return std::nullopt;
    }

    // Checking if exists, creating if it's not
    if (!fs::exists(todo_dir_path.value())) {
      cerr << "No ~/.todo directory, creating...";
      std::error_code ec;
      fs::create_directory(todo_dir_path.value(), ec);
      if (ec) {
        cerr << "ERROR: Unable to create directory: " << ec.message() << '\n';
        return std::nullopt;
      }
    }

    todo_db = std::make_unique<TodoDBFs>(todo_dir_path.value());
  } else {
    auto todo_data_str = string_view(todo_data);
    size_t possible_colon = todo_data_str.find(":");
    if (possible_colon != string_view::npos) {
      // Net path supplied
      auto address = todo_data_str.substr(0, possible_colon);
      size_t port = std::strtoull(
          string(todo_data_str.substr(possible_colon + 1)).c_str(), NULL, 10);
      auto endpoint =
          asio::ip::tcp::endpoint(asio::ip::make_address(address), port);
      todo_db = std::make_unique<TodoDBNetClient>(io, endpoint);

    } else {
      // Fs path supplied
      auto todo_dir_path = fs::path(todo_data_str);
      if (!fs::exists(todo_dir_path)) {
        cerr << "ERROR: Non existent directory supplied in environment "
                "variable\n";
        return std::nullopt;
      }
      todo_db = std::make_unique<TodoDBFs>(todo_dir_path);
    }
  }
  return todo_db;
}

void list_todos(const TodoDB &todo_db) {
  size_t todo_num = 1;
  todo_db.list();
  for (const auto &[name, status] : todo_db.list()) {
    string_view prefix;
    if (status == Status::InProgress)
      prefix = "> ";
    else if (status == Status::Done)
      prefix = "V ";
    cout << todo_num << ". " << prefix << name << '\n';
    todo_num++;
  }
}

void add_todo(const TodoDB &todo_db) {
  string name;
  while (true) {
    cout << "Name: ";
    if (!std::getline(cin, name))
      return;
    if (name.empty()) {
      cout << "Name can not be empty\n";
      continue;
    }
    if (todo_db.exists(name)) {
      cout << "This name already exists, try another\n";
      continue;
    }
    break;
  }
  cout << "Content (Ctrl+D to done):\n";
  std::ostringstream content_ss;
  content_ss << cin.rdbuf();

  const auto todo = Todo(std::move(name), content_ss.str(), Status::Undone);
  todo_db.save(todo);
}

void get_todo(const TodoDB &todo_db, size_t num) {
  const auto todos = todo_db.list();
  if (num > todos.size()) {
    cout << "No such entry\n";
    return;
  }
  const string_view name = todos[num - 1].first;
  const auto todo = todo_db.load(name);
  cout << todo.content();
}

void rm_todo(const TodoDB &todo_db, size_t num) {
  const auto todos = todo_db.list();
  if (num > todos.size()) {
    cout << "No such entry\n";
    return;
  }
  const string_view name = todos[num - 1].first;
  todo_db.remove(name);
}

void set_status(const TodoDB &todo_db, size_t num, Status sts) {
  const auto todos = todo_db.list();
  if (num > todos.size()) {
    cout << "No such entry\n";
    return;
  }
  const string_view name = todos[num - 1].first;
  auto todo = todo_db.load(name);
  todo.set_status(sts);
  todo_db.save(todo);
}
