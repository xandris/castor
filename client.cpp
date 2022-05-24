#include "client.hpp"

#include "uri.hpp"

Client::Client(Server *server, ssl_socket &&peer)
    : server{server},
      peer{std::move(peer)},
      _timeout{peer.get_executor(), 10s} {}

awaitable<void> Client::run() {
  auto ip = peer.next_layer().remote_endpoint();
  cout << ip << " Connected" << endl;

  try {
    co_await peer.async_handshake(ssl::stream_base::server);
    string b;
    auto sz =
        co_await async_read_until(peer, asio::dynamic_buffer(b, 1026), "\r\n");
    b.erase(b.end() - 2, b.end());
    string_view req{b.begin(), b.begin() + sz - 2};
    url::Uri u{req};
    if (u.scheme() != "gemini") {
      throw std::invalid_argument("Scheme must be 'gemini'");
    }
    cout << ip << " Host: " << u.host() << endl;
    cout << ip << " Path: " << u.path() << endl;
  } catch (const std::logic_error &e) {
    cout << ip << " Error: " << e.what() << endl;
  } catch (const system_error &e) {
    if (e.code().value() != asio::error::operation_aborted) {
      cout << ip << " Error: " << e.what() << endl;
    }
  }

  cout << "Closing " << ip << endl;
}

awaitable<void> Client::timeout() { co_await _timeout.async_wait(); }

void Client::close() {
  _timeout.cancel();
  peer.lowest_layer().close();
}