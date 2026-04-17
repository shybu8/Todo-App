#include "todo.hpp"
#include <string_view>

using std::string;
using std::string_view;

Todo::Todo(string name, string content, Status status) noexcept
    : name_(std::move(name)), content_(std::move(content)), status_(status) {}

void Todo::set_name(std::string name) noexcept { name_ = std::move(name); }

void Todo::set_content(std::string content) noexcept {
  content_ = std::move(content);
}

void Todo::set_status(Status status) noexcept { status_ = status; }

std::string_view Todo::name() const noexcept { return string_view(name_); }

std::string_view Todo::content() const noexcept {
  return string_view(content_);
}

Status Todo::status() const noexcept { return status_; }
