#pragma once

#include "enums_literals.hpp"
#include <optional>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace Protocol {
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

std::optional<std::variant<ListReq, RemoveReq, LoadReq, SaveReq>>
parse_req(std::string_view req);

std::string make_list_ans(std::vector<std::pair<std::string, Status>> todos);

std::string make_load_ans(std::string_view content, Status status);
// std::optional<std::string_view> parse_remove(std::string_view req);

} // namespace Server

namespace Client {

std::string make_save_req(std::string_view name, std::string_view content,
                          Status status);

std::string make_load_req(std::string_view name);

std::string make_remove_req(std::string_view name);

std::string make_list_req();

std::optional<std::pair<std::string_view, Status>>
parse_load_ans(std::string_view body);

std::optional<std::vector<std::pair<std::string, Status>>>
parse_list_ans(std::string_view body);

} // namespace Client

} // namespace Protocol
