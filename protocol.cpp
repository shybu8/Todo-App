#include "protocol.hpp"
#include "enums_literals.hpp"
#include <optional>
#include <string>
#include <utility>
#include <vector>

using std::optional;
using std::pair;
using std::string;
using std::string_view;
using std::variant;
using std::vector;

bool parse_line(string_view body, string_view &line, string_view &reminder) {
  size_t nl = body.find("\n");
  if (nl == string_view::npos)
    return false;
  line = body.substr(0, nl);
  if (line.empty())
    return false;
  reminder = body.substr(nl + 1);
  return true;
}

namespace Protocol {
namespace Server {

bool parse_remove(string_view body, RemoveReq &res) {
  string_view reminder;
  if (!parse_line(body, res.name, reminder))
    return false;
  return reminder == "\n";
}

bool parse_load(string_view body, LoadReq &res) {
  string_view reminder;
  if (!parse_line(body, res.name, reminder))
    return false;
  return reminder == "\n";
}

bool parse_save(string_view body, SaveReq &res) {
  if (!parse_line(body, res.name, body))
    return false;
  string_view status_str;
  if (!parse_line(body, status_str, body))
    return false;
  res.status = parse_status(status_str).value_or(Status::Undone);
  size_t end = body.find("\n\n");
  if (end == string_view::npos)
    return false;
  res.content = body.substr(0, end);
  return true;
}

optional<variant<ListReq, RemoveReq, LoadReq, SaveReq>>
parse_req(string_view req) {
  size_t nl = req.find("\n");
  if (nl == string_view::npos)
    return std::nullopt;
  auto req_kind = string_view(req.data(), nl);
  auto body = string_view(req.data() + nl + 1, req.length() - nl - 1);

  if (req_kind.empty())
    return std::nullopt;
  else if (req_kind == protocolCommandLiterals[protocol_command_to_index(
                           ProtocolCommand::Remove)]) {
    RemoveReq res;
    if (!parse_remove(body, res))
      return std::nullopt;
    return res;
  } else if (req_kind == protocolCommandLiterals[protocol_command_to_index(
                             ProtocolCommand::List)]) {
    ListReq res;
    return res;
  } else if (req_kind == protocolCommandLiterals[protocol_command_to_index(
                             ProtocolCommand::Load)]) {
    LoadReq res;
    if (!parse_load(body, res))
      return std::nullopt;
    return res;
  } else if (req_kind == protocolCommandLiterals[protocol_command_to_index(
                             ProtocolCommand::Save)]) {
    SaveReq res;
    if (!parse_save(body, res))
      return std::nullopt;
    return res;
  }

  return std::nullopt;
}

string make_list_ans(vector<pair<string, Status>> todos) {
  string ans;
  for (const auto &[name, status] : todos) {
    ans.append(name);
    ans.append(",");
    ans.append(statusLiterals[status_to_index(status)]);
    ans.append("\n");
  }
  ans.append("\n");
  return ans;
}

string make_load_ans(string_view content, Status status) {
  string buf;
  buf.append(statusLiterals[status_to_index(status)]);
  buf.append("\n");
  buf.append(content);
  buf.append("\n\n");
  return buf;
}

} // namespace Server

namespace Client {

string make_save_req(string_view name, string_view content, Status status) {
  string buf;
  buf.append(protocolCommandLiterals[protocol_command_to_index(
      ProtocolCommand::Save)]);
  buf.append("\n");
  buf.append(name);
  buf.append("\n");
  buf.append(statusLiterals[status_to_index(status)]);
  buf.append("\n");
  buf.append(content);
  buf.append("\n\n");
  return buf;
}

string make_load_req(string_view name) {
  string buf;
  buf.append(protocolCommandLiterals[protocol_command_to_index(
      ProtocolCommand::Load)]);
  buf.append("\n");
  buf.append(name);
  buf.append("\n\n");
  return buf;
}

string make_remove_req(string_view name) {
  string buf;
  buf.append(protocolCommandLiterals[protocol_command_to_index(
      ProtocolCommand::Remove)]);
  buf.append("\n");
  buf.append(name);
  buf.append("\n\n");
  return buf;
}

string make_list_req() {
  string buf;
  buf.append(protocolCommandLiterals[protocol_command_to_index(
      ProtocolCommand::List)]);
  buf.append("\n\n");
  return buf;
}

optional<pair<string_view, Status>> parse_load_ans(string_view body) {
  string_view status_str;
  if (!parse_line(body, status_str, body))
    return std::nullopt;
  size_t end = body.find("\n\n");
  if (end == string_view::npos)
    return std::nullopt;
  return std::make_pair(body.substr(0, end),
                        parse_status(status_str).value_or(Status::Undone));
}

optional<vector<pair<string, Status>>> parse_list_ans(string_view body) {
  vector<pair<string, Status>> res;
  size_t pos = 0;
  while (true) {
    size_t nl = body.find("\n", pos);
    if (nl == string::npos)
      break;

    auto line = string_view(body.data() + pos, nl - pos);
    if (line.empty())
      break;

    size_t comma = line.find(",");
    auto name = string_view(line.data(), comma);
    auto status_str =
        string_view(line.data() + comma + 1, line.length() - comma - 1);
    Status status = parse_status(status_str).value_or(Status::Undone);

    res.push_back(std::make_pair(string(name), status));

    pos = nl + 1;
  }
  auto end = body.substr(body.length() - 2, 2);
  if (end != "\n\n")
    return std::nullopt;
  return res;
}

} // namespace Client

} // namespace Protocol
