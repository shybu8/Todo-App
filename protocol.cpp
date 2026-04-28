#include "protocol.hpp"
#include "enums_literals.hpp"
#include <asio.hpp>
#include <asio/completion_condition.hpp>
#include <asio/impl/read_until.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using asio::ip::tcp;
using std::error_code;
using std::pair;
using std::string;
using std::string_view;
using std::variant;
using std::vector;

void parse_line(string_view body, string_view &line, string_view &reminder) {
  size_t nl = body.find("\n");
  if (nl == string_view::npos) {
    line = body;
    reminder = body.substr(body.length(), 0);
  } else {
    line = body.substr(0, nl);
    reminder = body.substr(nl + 1);
  }
}

void insert_content_len(string &msg) {
  size_t msg_len = msg.length();
  string msg_len_str = std::to_string(msg_len);
  msg_len_str.append("\n");
  msg.insert(0, msg_len_str);
}

namespace Protocol {

error_code read_message(tcp::socket &socket, string &msg,
                        string_view &headless_msg, asio::error_code &asio_ec) {
  asio::read_until(socket, asio::dynamic_buffer(msg), '\n', asio_ec);
  if (asio_ec)
    return Errc::AsioError;

  char *end;
  size_t target_body_len = std::strtoull(msg.data(), &end, 10);
  if (*end != '\n')
    return Errc::BadMessageLen;
  size_t body_len = msg.size() - (end - msg.data() + 1);

  if (body_len < target_body_len) {
    asio::read(socket, asio::dynamic_buffer(msg),
               asio::transfer_exactly(target_body_len - body_len), asio_ec);
    if (asio_ec)
      return Errc::AsioError;
  }

  headless_msg = string_view(end + 1, target_body_len);
  return {};
}

namespace Server {

error_code parse_remove_req(string_view body, RemoveReq &res) {
  string_view reminder;
  parse_line(body, res.name, reminder);
  return reminder == "" ? std::error_code{} : Errc::BadRequest;
}

error_code parse_load_req(string_view body, LoadReq &res) {
  string_view reminder;
  parse_line(body, res.name, reminder);
  return reminder == "" ? std::error_code{} : Errc::BadRequest;
}

error_code parse_save_req(string_view body, SaveReq &res) {
  parse_line(body, res.name, body);
  if (body == "")
    return Errc::BadRequest;
  string_view status_str;
  parse_line(body, status_str, body);
  res.status = parse_status(status_str).value_or(Status::Undone);
  res.content = body;
  return {};
}

variant<ListReq, RemoveReq, LoadReq, SaveReq> parse_req(string_view req,
                                                        error_code &ec) {
  string_view req_kind;
  string_view body;
  parse_line(req, req_kind, body);

  if (req_kind.empty()) {
    ec = Errc::BadRequest;
  } else if (req_kind == protocolCommandLiterals[protocol_command_to_index(
                             ProtocolCommand::Remove)]) {
    RemoveReq res;
    ec = parse_remove_req(body, res);
    return res;
  } else if (req_kind == protocolCommandLiterals[protocol_command_to_index(
                             ProtocolCommand::List)]) {
    ListReq res;
    return res;
  } else if (req_kind == protocolCommandLiterals[protocol_command_to_index(
                             ProtocolCommand::Load)]) {
    LoadReq res;
    ec = parse_load_req(body, res);
    return res;
  } else if (req_kind == protocolCommandLiterals[protocol_command_to_index(
                             ProtocolCommand::Save)]) {
    SaveReq res;
    ec = parse_save_req(body, res);
    return res;
  }
  return {};
}

void make_list_ans(string &ans, vector<pair<string, Status>> todos) {
  for (const auto &[name, status] : todos) {
    ans.append(name);
    ans.append(",");
    ans.append(statusLiterals[status_to_index(status)]);
    ans.append("\n");
  }
  insert_content_len(ans);
}

void make_load_ans(string &ans, string_view content, Status status) {
  ans.append(statusLiterals[status_to_index(status)]);
  ans.append("\n");
  ans.append(content);
  insert_content_len(ans);
}

} // namespace Server

namespace Client {

void make_save_req(string &req, string_view name, string_view content,
                   Status status) {
  req.append(protocolCommandLiterals[protocol_command_to_index(
      ProtocolCommand::Save)]);
  req.append("\n");
  req.append(name);
  req.append("\n");
  req.append(statusLiterals[status_to_index(status)]);
  req.append("\n");
  req.append(content);
  insert_content_len(req);
}

void make_load_req(string &req, string_view name) {
  req.append(protocolCommandLiterals[protocol_command_to_index(
      ProtocolCommand::Load)]);
  req.append("\n");
  req.append(name);
  insert_content_len(req);
}

void make_remove_req(string &req, string_view name) {
  req.append(protocolCommandLiterals[protocol_command_to_index(
      ProtocolCommand::Remove)]);
  req.append("\n");
  req.append(name);
  insert_content_len(req);
}

void make_list_req(string &req) {
  req.append(protocolCommandLiterals[protocol_command_to_index(
      ProtocolCommand::List)]);
  insert_content_len(req);
}

pair<string_view, Status> parse_load_ans(const string_view body,
                                         error_code &ec) {
  string_view status_str;
  string_view content;
  parse_line(body, status_str, content);
  if (status_str == "") {
    ec = Errc::BadAnswer;
    return {};
  }
  return std::make_pair(content,
                        parse_status(status_str).value_or(Status::Undone));
}

vector<pair<string, Status>> parse_list_ans(string_view body, error_code &ec) {
  vector<pair<string, Status>> res;
  (void)ec;
  while (true) {
    string_view line;
    parse_line(body, line, body);
    if (line.empty())
      break;

    size_t comma = line.find(",");
    auto name = string_view(line.data(), comma);
    auto status_str =
        string_view(line.data() + comma + 1, line.length() - comma - 1);
    Status status = parse_status(status_str).value_or(Status::Undone);

    res.push_back(std::make_pair(string(name), status));
  }
  return res;
}

} // namespace Client

} // namespace Protocol
