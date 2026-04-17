#pragma once

#include "enums_literals.hpp"
#include "todo_db.hpp"
#include <filesystem>
#include <optional>

std::optional<std::filesystem::path> todo_dir_path_opt();

void list_todos(const TodoDB &todo_db);

void add_todo(const TodoDB &todo_db);

void get_todo(const TodoDB &todo_db, size_t num);

void rm_todo(const TodoDB &todo_db, size_t num);

void set_status(const TodoDB &todo_db, size_t num, Status sts);
