#include <array>
#include <filesystem>
#include <optional>
#include <string_view>

namespace fs = std::filesystem;

enum class Command : unsigned { Add = 0, List, CommandListLen };

constexpr unsigned command_to_index(Command cmd) {
  return static_cast<unsigned>(cmd);
}

inline constexpr std::array<std::string_view,
                            command_to_index(Command::CommandListLen)>
    commandLiterals{
        "add",
        "list",
    };

inline constexpr std::string_view
    usageMessage("USAGE:\n\ttodo-app (add | list)\n\n");

std::optional<Command> parse_command(char *arg);

std::optional<fs::path> todo_dir_path_opt();
