#pragma once

#include "todo.hpp"
#include <asio.hpp>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class TodoDB {
public:
  virtual void save(const Todo &todo) const = 0;
  virtual Todo load(std::string_view name) const = 0;
  virtual void remove(std::string_view name) const = 0;
  virtual std::vector<std::pair<std::string, Status>> list() const = 0;
  virtual bool exists(std::string_view name) const = 0;
  virtual ~TodoDB() = default;
};

class TodoDBFs : public TodoDB {
  std::filesystem::path todo_dir_;

public:
  TodoDBFs(std::filesystem::path todo_dir) noexcept;
  void save(const Todo &todo) const override;
  Todo load(std::string_view name) const override;
  void remove(std::string_view name) const override;
  std::vector<std::pair<std::string, Status>> list() const override;
  bool exists(std::string_view name) const override;
};

class TodoDBNetClient : public TodoDB {
  mutable asio::ip::tcp::socket socket_;

public:
  TodoDBNetClient(asio::io_context &io,
                  const asio::ip::tcp::endpoint &endpoint);
  void save(const Todo &todo) const override;
  Todo load(std::string_view name) const override;
  void remove(std::string_view name) const override;
  std::vector<std::pair<std::string, Status>> list() const override;
  bool exists(std::string_view name) const override;
};
