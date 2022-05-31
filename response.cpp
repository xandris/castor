#include "response.hpp"

#include <array>
#include <cstdlib>

Response::Response(ssl_socket& _s) : socket{_s} {}

const char CRLF[]={'\r','\n'};

awaitable<void> Response::flush_header() {
  if (committed) co_return;
  auto [cat, code] = std::div(static_cast<int>(this->code), 10);

  array<char, 3> codebuf{static_cast<char>('0' + cat), static_cast<char>('0' + code), ' '};

  auto buffers = {asio::buffer(codebuf.cbegin(), codebuf.size()),
                  asio::buffer(meta),
                  asio::buffer(CRLF)};

  co_await asio::async_write(socket, buffers);
  committed = true;
}

void Response::header(code_t c, string_view m) {
  code = c;
  meta = m;
}