#pragma once

#include "enums_literals.hpp"
#include <asio.hpp>
#include <string_view>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

namespace Protocol {

enum class Errc {
  BadMessageLen = 1,
  BadRequest,
  BadAnswer,
  AsioError,
};

class ErrorCategory : public std::error_category {
public:
  const char *name() const noexcept override { return "protocol"; }
  std::string message(int ev) const override {
    switch (static_cast<Errc>(ev)) {
    case Errc::BadMessageLen:
      return "bad protocol message length";
    case Errc::BadRequest:
      return "bad protocol request";
    case Errc::BadAnswer:
      return "bad protocol answer";
    case Errc::AsioError:
      return "asio error";
    }

    return "unknown protocol error";
  }
};

inline const std::error_category &error_category() {
  static ErrorCategory category;
  return category;
}

inline std::error_code make_error_code(Errc e) {
  return {static_cast<int>(e), error_category()};
}

std::error_code read_message(asio::ip::tcp::socket &socket, std::string &msg,
                             std::string_view &headless_msg,
                             asio::error_code &asio_ec);

namespace Server {

struct RemoveReq {
  std::string_view name;
};

struct ListReq {};

struct LoadReq {
  std::string_view name;
};

struct SaveReq {
  std::string_view name;
  Status status;
  std::string_view content;
};

std::variant<ListReq, RemoveReq, LoadReq, SaveReq>
parse_req(std::string_view req, std::error_code &ec);

void make_list_ans(std::string &ans,
                   std::vector<std::pair<std::string, Status>> todos);

void make_load_ans(std::string &ans, std::string_view content, Status status);

} // namespace Server

namespace Client {

void make_save_req(std::string &req, std::string_view name,
                   std::string_view content, Status status);

void make_load_req(std::string &req, std::string_view name);

void make_remove_req(std::string &req, std::string_view name);

void make_list_req(std::string &req);

std::pair<std::string_view, Status> parse_load_ans(std::string_view body,
                                                   std::error_code &ec);

std::vector<std::pair<std::string, Status>>
parse_list_ans(std::string_view body, std::error_code &ec);

} // namespace Client

} // namespace Protocol

template <> struct std::is_error_code_enum<Protocol::Errc> : std::true_type {};
