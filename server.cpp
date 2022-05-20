#include "client.hpp"
#include "server.hpp"

#include "net-types.hpp"

Server::Server(ssl::context&& ctx) : io{}, ssl_context{std::move(ctx)} {}

void Server::run() {
  co_spawn(io, do_run(tcp::endpoint{tcp::v4(), 1965}), detached);

  io.run();
}

awaitable<void> Server::do_run(const tcp::endpoint ep) {
  auto sock = acceptor{io, ep};

  cout << "Listening for connections on " << ep << endl;

  try {
    for (;;) {
      ssl_socket peer{io, ssl_context};
      co_await sock.async_accept(peer.lowest_layer());
      auto client = std::make_shared<Client>(this, std::move(peer));
      clients.insert(client);
      co_spawn(io, client->run() || timeout(10s), [&](std::exception_ptr e, auto){
          // do nothing i guess
      });
    }
  } catch (const std::exception& e) {
    cout << "Error accepting: " << e.what() << endl;
    throw;
  }
}

awaitable<void> Server::timeout(timer::duration d) {
  co_await timer{io, d}.async_wait();
};
