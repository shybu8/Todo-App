#pragma once

#include <array>
#include <optional>
#include <string_view>

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
        "add", "ls", "get", "rm", "set",
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
    usageMessage("USAGE:\n\ttodo-app (add|ls|get NUM|rm NUM|set NUM "
                 "(undone|in_progress|done))\n\n");

inline std::optional<Command> parse_command(const char *arg) {
  std::string_view cmd(arg);
  for (size_t i = 0; i < command_to_index(Command::CommandListLen); i++) {
    if (cmd == commandLiterals[i])
      return static_cast<Command>(i);
  }
  return std::nullopt;
}

inline std::optional<Status> parse_status(const char *arg) {
  std::string_view cmd(arg);
  for (size_t i = 0; i < status_to_index(Status::StatusListLen); i++) {
    if (cmd == statusLiterals[i])
      return static_cast<Status>(i);
  }
  return std::nullopt;
}
