#pragma once

#include "enums_literals.hpp"
#include <string>
#include <string_view>

class Todo {
  std::string name_;
  std::string content_;
  Status status_;

public:
  Todo(std::string name, std::string content, Status status) noexcept;
  void set_name(std::string name) noexcept;
  void set_content(std::string content) noexcept;
  void set_status(Status status) noexcept;

  std::string_view name() const noexcept;
  std::string_view content() const noexcept;
  Status status() const noexcept;
};
