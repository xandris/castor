#include "dir.hpp"

#include <boost/asio/basic_file.hpp>

using basic_stream_file = asio::basic_stream_file<executor>;

DirHandler::DirHandler(std::filesystem::path p) : root{p} {}

awaitable<void> DirHandler::operator()(const Request &req, Response &res) {
  auto child = root / req.path_info.relative_path();
  /*
  if (std::filesystem::is_directory(child)) {
    res.header(Response::code_t::redirect_permanent, url::Uri{});
    co_await res.flush_header();
    co_return;
  }
  */

  if (!child.has_filename()) child /= "index.gmi";

  basic_stream_file f{res.socket.get_executor()};
  try {
    f.open(child, basic_stream_file::read_only);
    res.header(Response::code_t::success, "text/gemini");
  } catch (const system_error &e) {
    res.header(Response::code_t::not_found, "Not found.");
  }

  co_await res.flush_header();

  if (f.is_open()) {
    auto buffers = {
        asio::buffer("Hello, world!\n"),
        asio::buffer("You asked for: "),
        asio::buffer(child.native()),
        asio::buffer("\n"),
    };
    co_await async_write(res.socket, buffers);
    f.close();
  }
}