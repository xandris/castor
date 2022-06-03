#pragma once

#include "net-types.hpp"

struct Response {
  enum class category {
    input = 1,
    success,
    redirect,
    temporary_failure,
    permanent_failure,
    client_certificate_required,
  };

  enum class code_t {
    input = 10,
    sensitive_input,
    success = 20,
    redirect = 30,
    redirect_permanent,
    temporary_failure = 40,
    server_unavailable,
    cgi_error,
    proxy_error,
    slow_down,
    permanent_failure = 50,
    not_found,
    gone,
    proxy_request_refused,
    bad_request = 59,
    client_certificate_required = 60,
    certificate_not_authorised,
    certificate_not_valid,
  };

  Response(ssl_socket&);

  ssl_socket& socket;
  code_t code;
  string meta;

  void header(code_t, string_view);

  // TODO: automate committing the response.  Need Response to be async
  // writeable.  As it is, Handler can write to this->socket interleaved with
  // methods of this.
  awaitable<void> flush_header();

 private:
  bool committed{};
};