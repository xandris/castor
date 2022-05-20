#include "client.hpp"

Client::Client(Server *server, ssl_socket &&peer)
    : server{server}, peer{std::move(peer)} {}

awaitable<void> Client::run() {
  auto ip = peer.next_layer().remote_endpoint();
  cout << ip << " Connected" << endl;

  try {
    co_await peer.async_handshake(ssl::stream_base::server);
    string b;
    auto sz =
        co_await async_read_until(peer, asio::dynamic_buffer(b, 1026), "\r\n");
    b.erase(b.end() - 2, b.end());
    string_view req{b.begin(), b.begin()+sz-2};
    cout << ip << " Request: " << req << endl;
  } catch (const std::exception &e) {
    cout << ip << " Error: " << e.what() << endl;
  }
  cout << "Closing " << peer.next_layer().remote_endpoint() << endl;
}