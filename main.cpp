#include <filesystem>

#include "net-types.hpp"
#include "request.hpp"
#include "response.hpp"
#include "server.hpp"

int main(int argc, char *argv[]) {
  try {
    ssl::context ssl_context(ssl::context::tlsv13_server);
    ssl_context.use_certificate_file("certs/cert.pem", ssl::context::pem);
    ssl_context.use_private_key_file("certs/privkey.pem", ssl::context::pem);

    std::map<std::filesystem::path, Handler> handlers{
        {"/asdf", Handler{[](const Request &req, Response &res) -> awaitable<void> {
           res.header(Response::code_t::success, "text/gemini");
           co_await res.flush_header();
           auto buffers = {asio::buffer("Hello, world! You asked for:\n"),
                           asio::buffer(req.path_info.native())};
           co_await asio::async_write(res.socket, buffers);
         }}}};
    Server server{std::move(ssl_context), handlers};
    server.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << endl;
    return 1;
  }

  return 0;
}
