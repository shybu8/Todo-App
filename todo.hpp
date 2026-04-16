#include <array>
#include <filesystem>
#include <optional>
#include <string_view>

namespace fs = std::filesystem;

enum class Command : std::size_t {
  Add = 0,
  List,
  Get,
  Remove,
  Set,
  CommandListLen
};

constexpr size_t command_to_index(Command cmd) {
  return static_cast<std::size_t>(cmd);
}

inline constexpr std::array<std::string_view,
                            command_to_index(Command::CommandListLen)>
    commandLiterals{
        "add", "list", "get", "rm", "set",
    };

enum class Status : std::size_t { Undone = 0, InProgress, Done, StatusListLen };

constexpr size_t status_to_index(Status sts) {
  return static_cast<std::size_t>(sts);
}

inline constexpr std::array<std::string_view,
                            status_to_index(Status::StatusListLen)>
    statusLiterals{
        "undone",
        "in_progress",
        "done",
    };

inline constexpr std::string_view
    usageMessage("USAGE:\n\ttodo-app (add|list|get NUM|rm NUM|set NUM "
                 "(undone|in_progress|done))\n\n");

std::optional<Command> parse_command(const char *arg);

std::optional<Status> parse_status(const char *arg);

std::optional<fs::path> todo_dir_path_opt();

void list_todos(const fs::path &todo_dir_path);

void add_todo(const fs::path &todo_dir_path);

void get_todo(const fs::path &todo_dir_path, size_t num);

void rm_todo(const fs::path &todo_dir_path, size_t num);

void set_status(const fs::path &todo_dir_path, size_t num, Status sts);

struct Todo {
  std::string name;
  std::string content;
  Status status;

  Todo(std::string name, std::string content, Status status) noexcept;
  Todo(const fs::path &todo_file);

  void save(const fs::path &todo_dir) const;
};
