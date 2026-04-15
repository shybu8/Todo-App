#include "todo.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>

using std::cerr;
using std::cin;
using std::optional;

int main(int argc, char *argv[]) {
  optional<fs::path> todo_dir_path = todo_dir_path_opt();
  if (!todo_dir_path.has_value()) {
    cerr << "ERROR: Unable to get homedir path";
    return 1;
  }

  if (!fs::exists(todo_dir_path.value())) {
    cerr << "No ~/.todo directory, creating...";
    std::error_code ec;
    fs::create_directory(todo_dir_path.value(), ec);
    if (ec) {
      cerr << ec.message() << '\n';
      return 1;
    }
  }

  optional<Command> cmd;
  cmd = argc > 1 ? parse_command(argv[1]) : std::nullopt;
  if (!cmd.has_value()) {
    cerr << usageMessage;
    return 1;
  }

  switch (cmd.value()) {
  case Command::Add:
    add_todo(todo_dir_path.value());
    break;
  case Command::List:
    list_todos(todo_dir_path.value());
    break;
  case Command::Get: {
    if (argc < 3) {
      cerr << "ERROR: Number of entry isn't supplied (see list command)\n";
      return 1;
    }
    char *end;
    size_t entry_num = std::strtoull(argv[2], &end, 10);
    if (argv[2] == end || *end != '\0') {
      cerr << "ERROR: Supplied entry number is invalid\n";
      return 1;
    }
    get_todo(todo_dir_path.value(), entry_num);
  } break;
  case Command::CommandListLen:
    break;
  }

  return 0;
}
