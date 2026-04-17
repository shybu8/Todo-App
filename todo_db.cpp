#include "todo_db.hpp"
#include "enums_literals.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <utility>

namespace fs = std::filesystem;
using std::cerr;
using std::optional;
using std::pair;
using std::string;
using std::vector;

TodoDBFs::TodoDBFs(fs::path todo_dir) noexcept : todo_dir_(todo_dir) {}

void TodoDBFs::save(const Todo &todo) const {
  const fs::path file_path = todo_dir_ / todo.name();
  std::ofstream file(file_path, std::ios::trunc);
  if (!file)
    throw std::runtime_error("Unable to open file for writing");
  file << statusLiterals[status_to_index(todo.status())] << '\n';
  file << todo.content();
};

Todo TodoDBFs::load(std::string_view name) const {
  const fs::path file_path = todo_dir_ / name;
  std::ifstream file(file_path);
  if (!file)
    throw std::runtime_error("Unable to open file");

  string status_str;
  if (!std::getline(file, status_str)) {
    throw std::runtime_error("Unable to read todo status");
  }

  optional<Status> status = parse_status(status_str.c_str());
  if (!status) {
    cerr << "Unrecognized todo status, falling back to 'undone'\n";
    status = Status::Undone;
  }

  std::ostringstream content;
  content << file.rdbuf();

  return Todo(string(name), content.str(), status.value());
};

void TodoDBFs::remove(std::string_view name) const {
  const fs::path file_path = todo_dir_ / name;
  fs::remove(file_path);
};

vector<pair<string, Status>> TodoDBFs::list() const {
  vector<pair<string, Status>> res;
  for (const auto &entry : fs::directory_iterator(todo_dir_)) {
    if (entry.is_regular_file()) {
      std::ifstream file(entry.path());
      if (!file)
        throw std::runtime_error("Unable to open file for reading");
      string first_word;
      Status status;
      file >> first_word;
      if (first_word == statusLiterals[status_to_index(Status::InProgress)])
        status = Status::InProgress;
      else if (first_word == statusLiterals[status_to_index(Status::Done)])
        status = Status::Done;
      else
        status = Status::Undone;
      res.push_back({entry.path().filename().string(), status});
    }
  }
  return res;
};

bool TodoDBFs::exists(std::string_view name) const {
  for (const auto &entry : fs::directory_iterator(todo_dir_)) {
    if (entry.is_regular_file() && entry.path().filename() == name)
      return true;
  }
  return false;
}
