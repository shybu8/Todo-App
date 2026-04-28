#include "protocol.hpp"
#include "utils.hpp"
#include "visitor_pattern.hpp"
#include <asio.hpp>
#include <cstdlib>
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
using std::unique_ptr;

int main() {
  // optional<fs::path> todo_dir_path = default_todo_dir_path_opt();
  // if (!todo_dir_path.has_value()) {
  //   cerr << "ERROR: Unable to get homedir path";
  //   return 1;
  // }

  // if (!fs::exists(todo_dir_path.value())) {
  //   cerr << "No ~/.todo directory, creating...";
  //   std::error_code ec;
  //   fs::create_directory(todo_dir_path.value(), ec);
  //   if (ec) {
  //     cerr << "ERROR: Unable to create directory: " << ec.message() << '\n';
  //     return 1;
  //   }
  // }

  // const auto todo_db = TodoDBFs(todo_dir_path.value());

  asio::io_context io_ctx;
  char *todo_data = std::getenv("TODO_DATA");
  auto todo_db_opt = determine_backend(io_ctx, todo_data);
  if (!todo_db_opt)
    return 1;
  unique_ptr<TodoDB> todo_db = std::move(todo_db_opt.value());

  tcp::acceptor acceptor(io_ctx, tcp::endpoint(tcp::v4(), 9999));
  for (;;) {
    tcp::socket socket(io_ctx);
    acceptor.accept(socket);
    string buf;
    for (;;) {
      asio::error_code asio_ec;
      std::error_code std_ec;
      string_view req;
      std_ec = Protocol::read_message(socket, buf, req, asio_ec);
      if (std_ec == Protocol::Errc::AsioError && asio_ec == asio::error::eof)
        break;

      auto parsed = Protocol::Server::parse_req(req, std_ec);
      if (std_ec) {
        cerr << "Protocol violation: " << std_ec.message() << '\n';
        buf.clear();
        continue;
      }
      std::visit(
          overloaded{[&todo_db](Protocol::Server::RemoveReq remove_req) {
                       (*todo_db).remove(remove_req.name);
                     },
                     [&todo_db, &socket](Protocol::Server::ListReq list_req) {
                       (void)list_req;
                       string ans;
                       Protocol::Server::make_list_ans(ans, (*todo_db).list());
                       asio::error_code ignored_error;
                       asio::write(socket, asio::buffer(ans), ignored_error);
                     },
                     [&todo_db, &socket](Protocol::Server::LoadReq load_req) {
                       const auto todo = (*todo_db).load(load_req.name);
                       string ans;
                       Protocol::Server::make_load_ans(ans, todo.content(),
                                                       todo.status());
                       asio::write(socket, asio::buffer(ans));
                     },
                     [&todo_db](Protocol::Server::SaveReq save_req) {
                       auto todo =
                           Todo(string(save_req.name), string(save_req.content),
                                save_req.status);
                       (*todo_db).save(todo);
                     }},
          parsed);
      buf.clear();
    }
  }
  return 0;
}
