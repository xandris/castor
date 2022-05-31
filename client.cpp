#include "client.hpp"

#include "response.hpp"
#include "uri.hpp"

Client::Client(Server &server, ssl_socket &&peer)
    : server{server},
      peer{std::move(peer)},
      _timeout{peer.get_executor(), 10s} {}

awaitable<void> Client::run() {
  auto ip = peer.next_layer().remote_endpoint();
  cout << ip << " Connected" << '\n';

  try {
    co_await peer.async_handshake(ssl::stream_base::server);

    const char *err;
    Response res{peer};

    try {
      string b;
      auto sz = co_await async_read_until(peer, asio::dynamic_buffer(b, 1026),
                                          "\r\n");
      b.erase(b.end() - 2, b.end());
      url::Uri u{string_view{b.begin(), b.begin() + sz - 2}};
      if (u.scheme() != "gemini") {
        res.header(Response::code_t::bad_request,
                   "URL must start with 'gemini:'");
      } else if(!u.path().is_absolute()) {
        res.header(Response::code_t::bad_request,
                   "URL must be absolute.");
      } else {
        auto h = server.handler_for(u.path());
        if (h) {
          Request req{u, h->second};
          co_await h->first(req, res);
        } else {
          res.header(Response::code_t::not_found, "Not found.");
        }
      }
    } catch (const system_error &e) {
      switch (e.code().value()) {
        case asio::error::not_found:
          res.header(Response::code_t::bad_request, "Missing CRLF.");
          break;
        default:
          cout << ip << " system_error" << e.code().value() << endl;
          res.header(Response::code_t::bad_request, "Request is malformed.");
          break;
      }
    } catch (const std::logic_error &e) {
      res.header(Response::code_t::bad_request, "Request is malformed.");
    }

    co_await res.flush_header();

    cout << ip << ' ' << static_cast<int>(res.code) << ' ' << res.meta << '\n';
  } catch (const std::exception &e) {
    cout << ip << " Error: " << typeid(e).name() << ' ' << e.what() << '\n';
  }

  cout << "Closing " << ip << '\n';
}

awaitable<void> Client::timeout() { co_await _timeout.async_wait(); }

void Client::close() {
  _timeout.cancel();
  peer.lowest_layer().close();
}