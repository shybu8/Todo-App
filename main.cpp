#include "todo_db.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

using std::cerr;
using std::optional;
using std::size_t;
using std::string;
using std::string_view;
using std::unique_ptr;

optional<size_t> parse_argv_num(int argc, char *argv[], int target) {
  if (argc < target + 1) {
    cerr << "ERROR: Number of entry isn't supplied (see list command)\n";
    return std::nullopt;
  }
  char *end;
  size_t entry_num = std::strtoull(argv[target], &end, 10);
  if (argv[target] == end || *end != '\0') {
    cerr << "ERROR: Supplied entry number is invalid\n";
    return std::nullopt;
  }
  return entry_num;
}

int main(int argc, char *argv[]) {
  asio::io_context io;
  char *todo_data = std::getenv("TODO_DATA");
  auto todo_db_opt = determine_backend(io, todo_data);
  if (!todo_db_opt)
    return 1;
  unique_ptr<TodoDB> todo_db = std::move(todo_db_opt.value());

  optional<Command> cmd;
  cmd = argc > 1 ? parse_command(argv[1]) : std::nullopt;
  if (!cmd.has_value()) {
    cerr << usageMessage;
    return 1;
  }

  switch (cmd.value()) {
  case Command::Add:
    add_todo(*todo_db);
    break;
  case Command::List:
    list_todos(*todo_db);
    break;
  case Command::Get: {
    optional<size_t> target(parse_argv_num(argc, argv, 2));
    if (!target.has_value())
      return 1;
    get_todo(*todo_db, target.value());
  } break;
  case Command::Remove: {
    optional<size_t> target(parse_argv_num(argc, argv, 2));
    if (!target.has_value())
      return 1;
    rm_todo(*todo_db, target.value());
  } break;
  case Command::Set: {
    optional<size_t> target(parse_argv_num(argc, argv, 2));
    if (!target.has_value())
      return 1;
    if (argc < 4) {
      cerr << "Please supply status\nUSAGE:\n\ttodo-app set NUM STS\n\n";
      return 1;
    }
    optional<Status> status(parse_status(argv[3]));
    if (!status.has_value()) {
      cerr << "ERROR: Invalid status supplied, use (undone, in_progress, "
              "done)\n";
      return 1;
    }
    set_status(*todo_db, target.value(), status.value());
  } break;
  case Command::CommandListLen:
    break;
  }
  return 0;
}
