#include "dir.hpp"

#include <boost/asio/basic_file.hpp>

using basic_stream_file = asio::basic_stream_file<executor>;

DirHandler::DirHandler(std::filesystem::path p) : root{p} {}

awaitable<void> DirHandler::operator()(const Request &req, Response &res) {
  auto child = root / req.path_info.relative_path();
  cout << child << '\n';
  if (std::filesystem::is_directory(child) && req.uri.path().has_filename()) {
    auto dirpath = req.uri.path() / "";
    auto redirect = string(url::Uri{dirpath.native(), req.uri});
    res.header(Response::code_t::redirect_permanent, redirect);
    co_return;
  }

  if (!child.has_filename()) child /= "index.gmi";

  basic_stream_file f{res.socket.get_executor()};
  try {
    f.open(child, basic_stream_file::read_only);
    res.header(Response::code_t::success, "text/gemini");
  } catch (const system_error &e) {
    res.header(Response::code_t::not_found, "Not found.");
    co_return;
  }

  co_await res.flush_header();

  if (f.is_open()) {
    array<char, (1 << 18)> array;
    auto buf = asio::mutable_buffer{array.begin(), array.size()};
    try {
      for (;;) {
        auto sz = co_await f.async_read_some(buf, asio::use_awaitable);
        if (sz > 0)
          co_await async_write(res.socket,
                               asio::const_buffer{array.begin(), sz});
      }
    } catch (const system_error &e) {
      if (e.code() != asio::error::eof) {
        throw;
      }
    } catch (const std::exception &e) {
      cerr << "Exception serving file: " << e.what() << '\n';
    }
    f.close();
  }
}