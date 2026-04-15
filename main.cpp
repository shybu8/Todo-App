#include "todo.hpp"
#include <filesystem>
#include <iostream>

int main(int argc, char *argv[]) {
  std::optional<Command> cmd;
  cmd = argc > 1 ? parse_command(argv[1]) : std::nullopt;

  std::optional<fs::path> todo_dir_path = todo_dir_path_opt();
  if (!todo_dir_path.has_value()) {
    std::cerr << "ERROR: Unable to get homedir path";
    return 1;
  }

  if (!fs::exists(todo_dir_path.value())) {
    std::cerr << "No ~/.todo directory, creating...";
    std::error_code ec;
    fs::create_directory(todo_dir_path.value(), ec);
    if (ec) {
      std::cerr << ec.message() << '\n';
      return 1;
    }
  }

  if (!cmd.has_value()) {
    std::cerr << usageMessage;
    return 1;
  }

  switch (cmd.value()) {
  case Command::Add:
    std::cerr << "Not implemented\n";
    break;
  case Command::List:
    std::cerr << "Not implemented\n";
    break;
  case Command::CommandListLen:
    break;
  }

  return 0;
}
