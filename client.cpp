#include "client.hpp"

#include <variant>

#include "openssl.hpp"
#include "response.hpp"
#include "uri.hpp"

// Either a Request or an explanation why it couldn't parse.
using MaybeReq = std::variant<Request, string_view>;

awaitable<MaybeReq> parse_request(ssl_socket &peer) {
  try {
    string b;
    auto sz =
        co_await async_read_until(peer, asio::dynamic_buffer(b, 1026), "\r\n");
    b.erase(b.end() - 2, b.end());
    url::Uri u{string_view{b.begin(), b.begin() + sz - 2}};

    if (u.scheme() != "gemini") {
      co_return "URL must start with 'gemini:'";
    }

    if (!u.path().is_absolute()) {
      co_return "URL must be absolute.";
    }

    co_return Request{std::move(u)};
  } catch (const system_error &e) {
    switch (e.code().value()) {
      case asio::error::not_found:
        co_return "Missing CRLF.";
        break;
      default:
        cout << peer.lowest_layer().remote_endpoint() << " system_error"
             << e.code().value() << endl;
        co_return "Request is malformed.";
        break;
    }
  } catch (const std::logic_error &e) {
    co_return "Request is malformed.";
  }
}

Client::Client(Server &_server, ssl_socket &&_peer)
    : server{_server},
      peer{std::move(_peer)},
      _timeout{peer.get_executor(), 10s} {}

awaitable<void> Client::run() {
  auto ip = peer.next_layer().remote_endpoint();
  cout << ip << " Connected" << '\n';

  try {
    co_await peer.async_handshake(ssl::stream_base::server);
    string serverName;

    {
      openssl::Ssl ssl{peer.native_handle()};
      if (auto cstr{ssl.servername()}; cstr)
        serverName = cstr;
      else if (auto session{ssl.session()}; session)
        if (auto cstr{session.hostname()}; cstr) serverName = cstr;
    }

    Response res{peer};
    auto maybeReq = co_await parse_request(peer);
    if (maybeReq.index() == 0) {
      auto req = std::get<0>(maybeReq);
      auto h = server.handler_for(req.uri.path());
      if (h) {
        req.path_info = h->second;
        co_await h->first(req, res);
      } else {
        res.header(Response::code_t::not_found, "Not found.");
      }
    } else {
      res.header(Response::code_t::bad_request, std::get<1>(maybeReq));
    }

    co_await res.flush_header();

    cout << ip << ' ' << static_cast<int>(res.code) << ' ' << res.meta << '\n';
  } catch (const std::exception &e) {
    cout << ip << " Error: " << typeid(e).name() << ' ' << e.what() << '\n';
  }

  cout << "Closing " << ip << '\n';
  co_await peer.async_shutdown();
}

awaitable<void> Client::timeout() { co_await _timeout.async_wait(); }

void Client::close() {
  _timeout.cancel();
  peer.lowest_layer().close();
}