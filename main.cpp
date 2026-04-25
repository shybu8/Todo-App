#include "todo_db.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace fs = std::filesystem;
using std::cerr;
using std::optional;
using std::size_t;
using std::string;
using std::string_view;

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
  std::unique_ptr<TodoDB> todo_db;

  char *todo_data = std::getenv("TODO_DATA");
  if (todo_data == nullptr) {
    // Getting default dir path
    optional<fs::path> todo_dir_path = default_todo_dir_path_opt();
    if (!todo_dir_path.has_value()) {
      cerr << "ERROR: Unable to get homedir path";
      return 1;
    }

    // Checking if exists, creating if it's not
    if (!fs::exists(todo_dir_path.value())) {
      cerr << "No ~/.todo directory, creating...";
      std::error_code ec;
      fs::create_directory(todo_dir_path.value(), ec);
      if (ec) {
        cerr << "ERROR: Unable to create directory: " << ec.message() << '\n';
        return 1;
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
        return 1;
      }
      todo_db = std::make_unique<TodoDBFs>(todo_dir_path);
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
