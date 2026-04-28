#include "todo_db.hpp"
#include "enums_literals.hpp"
#include "protocol.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <utility>

namespace fs = std::filesystem;
using asio::ip::tcp;
using std::cerr;
using std::optional;
using std::pair;
using std::string;
using std::string_view;
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

  optional<Status> status = parse_status(status_str);
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

TodoDBNetClient::TodoDBNetClient(asio::io_context &io,
                                 const tcp::endpoint &endpoint)
    : socket_(io) {
  socket_.connect(endpoint);
}

void TodoDBNetClient::save(const Todo &todo) const {
  string buf;
  Protocol::Client::make_save_req(buf, todo.name(), todo.content(),
                                  todo.status());
  asio::error_code ec;
  asio::write(socket_, asio::buffer(buf), ec);
  if (ec)
    throw std::runtime_error(ec.message());
}

Todo TodoDBNetClient::load(std::string_view name) const {
  string buf;
  Protocol::Client::make_load_req(buf, name);
  asio::error_code ec;
  asio::write(socket_, asio::buffer(buf), ec);
  if (ec)
    throw std::runtime_error(ec.message());
  buf.clear();

  asio::error_code asio_ec;
  std::error_code std_ec;
  string_view body;
  std_ec = Protocol::read_message(socket_, buf, body, asio_ec);
  if (std_ec)
    throw std::runtime_error("Protocol violation");

  auto content_status = Protocol::Client::parse_load_ans(body, ec);
  if (std_ec)
    throw std::runtime_error("Protocol violation");
  auto todo =
      Todo(string(name), string(content_status.first), content_status.second);
  return todo;
}

void TodoDBNetClient::remove(std::string_view name) const {
  string buf;
  Protocol::Client::make_remove_req(buf, name);
  asio::error_code ec;
  asio::write(socket_, asio::buffer(buf), ec);
  if (ec)
    throw std::runtime_error(ec.message());
}

vector<pair<string, Status>> TodoDBNetClient::list() const {
  string buf;
  Protocol::Client::make_list_req(buf);
  asio::error_code asio_ec;
  asio::write(socket_, asio::buffer(buf), asio_ec);
  buf.clear();

  std::error_code std_ec;
  string_view body;
  std_ec = Protocol::read_message(socket_, buf, body, asio_ec);
  if (std_ec)
    throw std::runtime_error("Protocol violation");
  auto res = Protocol::Client::parse_list_ans(body, std_ec);
  if (std_ec)
    throw std::runtime_error("Protocol violation");
  return res;
}

bool TodoDBNetClient::exists(std::string_view target_name) const {
  auto pairs = list();
  for (const auto &[name, status] : pairs) {
    if (name == target_name)
      return true;
  }
  return false;
}
