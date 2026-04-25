#include "utils.hpp"
#include "todo.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace fs = std::filesystem;
using std::cin;
using std::cout;
using std::optional;
using std::string;
using std::string_view;

optional<fs::path> default_todo_dir_path_opt() {
  const string_view homeRelativePath = ".todo";

  char *home = std::getenv("HOME");
  if (!home)
    return std::nullopt;
  fs::path todo_path(home);
  todo_path.append(homeRelativePath);
  return todo_path;
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
