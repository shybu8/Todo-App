#include <array>
#include <filesystem>
#include <optional>
#include <string_view>

namespace fs = std::filesystem;

enum class Command : std::size_t { Add = 0, List, Get, Remove, CommandListLen };

constexpr unsigned command_to_index(Command cmd) {
  return static_cast<std::size_t>(cmd);
}

inline constexpr std::array<std::string_view,
                            command_to_index(Command::CommandListLen)>
    commandLiterals{
        "add",
        "list",
        "get",
        "rm",
    };

inline constexpr std::string_view
    usageMessage("USAGE:\n\ttodo-app (add | list | get | rm)\n\n");

std::optional<Command> parse_command(char *arg);

std::optional<fs::path> todo_dir_path_opt();

void list_todos(fs::path todo_dir_path);

void add_todo(fs::path todo_dir_path);

void get_todo(fs::path todo_dir_path, size_t num);

void rm_todo(fs::path todo_dir_path, size_t num);
