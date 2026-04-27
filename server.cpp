#include "protocol.hpp"
#include "utils.hpp"
#include "visitor_pattern.hpp"
#include <asio.hpp>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>

namespace fs = std::filesystem;
using asio::ip::tcp;
using std::cerr;
using std::optional;
using std::string;
using std::string_view;

int main() {
  optional<fs::path> todo_dir_path = default_todo_dir_path_opt();
  if (!todo_dir_path.has_value()) {
    cerr << "ERROR: Unable to get homedir path";
    return 1;
  }

  if (!fs::exists(todo_dir_path.value())) {
    cerr << "No ~/.todo directory, creating...";
    std::error_code ec;
    fs::create_directory(todo_dir_path.value(), ec);
    if (ec) {
      cerr << "ERROR: Unable to create directory: " << ec.message() << '\n';
      return 1;
    }
  }

  const auto todo_db = TodoDBFs(todo_dir_path.value());

  asio::io_context io_ctx;
  tcp::acceptor acceptor(io_ctx, tcp::endpoint(tcp::v4(), 9999));
  for (;;) {
    tcp::socket socket(io_ctx);
    acceptor.accept(socket);
    string buf;
    for (;;) {
      asio::error_code ec;
      asio::read_until(socket, asio::dynamic_buffer(buf), "\n\n", ec);
      if (ec == asio::error::eof)
        break;

      size_t req_end = buf.find("\n\n");
      auto req = string_view(buf.data(), req_end + 2);

      auto parsed = Protocol::Server::parse_req(req);
      if (!parsed.has_value()) {
        cerr << "Protocol violation\n";
        buf.erase(0, req_end + 2);
        continue;
      }
      std::visit(
          overloaded{[&todo_db](Protocol::Server::RemoveReq remove_req) {
                       todo_db.remove(remove_req.name);
                     },
                     [&todo_db, &socket](Protocol::Server::ListReq list_req) {
                       (void)list_req;
                       auto ans =
                           Protocol::Server::make_list_ans(todo_db.list());
                       asio::error_code ignored_error;
                       asio::write(socket, asio::buffer(ans), ignored_error);
                     },
                     [&todo_db, &socket](Protocol::Server::LoadReq load_req) {
                       const auto todo = todo_db.load(load_req.name);
                       auto buf = Protocol::Server::make_load_ans(
                           todo.content(), todo.status());
                       asio::write(socket, asio::buffer(buf));
                     },
                     [&todo_db](Protocol::Server::SaveReq save_req) {
                       auto todo =
                           Todo(string(save_req.name), string(save_req.content),
                                save_req.status);
                       todo_db.save(todo);
                     }},
          parsed.value());

      // size_t first_token_end = req.find('\n');
      // string first_token = string(req.substr(0, first_token_end));
      // optional<ProtocolCommand> cmd =
      //     parse_protocol_command(first_token.c_str());
      // if (!cmd) {
      //   buf.erase(0, req_end + 2);
      //   continue;
      // }
      // switch (cmd.value()) {
      // case ProtocolCommand::List: {
      // } break;
      // case ProtocolCommand::Load: {
      //   string ans;
      //   size_t second_token_end = req.find("\n", first_token_end + 1);
      //   size_t name_pos = first_token_end + 1;
      //   size_t name_len = second_token_end - first_token_end - 1;
      //   string_view name(req.data() + name_pos, name_len);
      //   const auto todo = todo_db.load(name);
      //   string buf;
      //   buf.append(statusLiterals[status_to_index(todo.status())]);
      //   buf.append("\n");
      //   buf.append(todo.content());
      //   buf.append("\n\n");
      //   asio::write(socket, asio::buffer(buf));
      // } break;
      // case ProtocolCommand::Remove: {
      //   size_t second_token_end = req.find("\n", first_token_end + 1);
      //   size_t name_pos = first_token_end + 1;
      //   size_t name_len = second_token_end - first_token_end - 1;
      //   string_view name(req.data() + name_pos, name_len);
      //   todo_db.remove(name);
      // } break;
      // case ProtocolCommand::Save: {
      //   size_t second_token_end = req.find("\n", first_token_end + 1);
      //   size_t name_pos = first_token_end + 1;
      //   size_t name_len = second_token_end - first_token_end - 1;
      //   size_t third_token_end = req.find("\n", second_token_end + 1);
      //   size_t status_pos = second_token_end + 1;
      //   size_t status_len = third_token_end - second_token_end - 1;
      //   string_view name(req.data() + name_pos, name_len);
      //   string_view status_str(req.data() + status_pos, status_len);
      //   optional<Status> status = parse_status(string(status_str).c_str());
      //   string_view content(req.data() + third_token_end + 1,
      //                       req.length() - third_token_end - 3);
      //   auto todo = Todo(string(name), string(content),
      //                    status.value_or(Status::Undone));
      //   todo_db.save(todo);
      // } break;
      // case ProtocolCommand::ProtocolCommandListLen:
      //   break;
      // }
      buf.erase(0, req_end + 2);
    }
  }
  return 0;
}
