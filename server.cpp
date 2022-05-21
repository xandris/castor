#include "server.hpp"

#include "client.hpp"
#include "net-types.hpp"

namespace {

string present_sockopt(const acceptor::linger& o) {
  if (o.enabled()) {
    return (std::stringstream{} << "yes, " << o.timeout() << " seconds").str();
  }
  return "no";
}

template <typename OptType>
requires requires(OptType o) { o.value(); }
auto present_sockopt(const OptType& o) { return o.value(); }

template <std::default_initializable OptType>
requires requires(OptType o) { cout << present_sockopt(o); }
void report_sock_opt(const acceptor& sock, string name) {
  OptType opt{};
  sock.get_option(opt);
  cout << '\t' << name << ": " << present_sockopt(opt) << '\n';
}

void report_sock_opts(const acceptor& sock) {
  cout << "socket options:\n";
  report_sock_opt<acceptor::broadcast>(sock, "broadcast");
  report_sock_opt<acceptor::debug>(sock, "debug");
  report_sock_opt<acceptor::do_not_route>(sock, "do_not_route");
  report_sock_opt<acceptor::enable_connection_aborted>(
      sock, "enable_connection_aborted");
  report_sock_opt<acceptor::keep_alive>(sock, "keep_alive");
  report_sock_opt<acceptor::linger>(sock, "linger");
  report_sock_opt<acceptor::receive_buffer_size>(sock, "receive_buffer_size");
  report_sock_opt<acceptor::receive_low_watermark>(sock,
                                                   "receive_low_watermark");
  report_sock_opt<acceptor::reuse_address>(sock, "reuse_address");
  report_sock_opt<acceptor::send_buffer_size>(sock, "send_buffer_size");
  report_sock_opt<acceptor::send_low_watermark>(sock, "send_low_watermark");
}

}  // namespace

Server::Server(ssl::context&& ctx)
    : io{}, ssl_context{std::move(ctx)}, sock{io} {}

void Server::run() {
  std::exception_ptr exc;
  co_spawn(io, do_run(tcp::endpoint{tcp::v4(), 1965}),
           [&](std::exception_ptr e) { exc = e; });

  io.run();
  shutdown();

  if (exc) {
    std::rethrow_exception(exc);
  }
}

awaitable<void> Server::do_run(const tcp::endpoint ep) {
  auto signals = asio::signal_set{io};
  signals.add(SIGTERM);
  signals.add(SIGINT);
  signals.async_wait(std::bind_front(&Server::on_signal, this));
  sock.open(ep.protocol());
  sock.set_option(acceptor::reuse_address(true));
  report_sock_opts(sock);
  sock.bind(ep);
  sock.listen();

  cout << "Listening for connections on " << ep << endl;

  try {
    for (;;) {
      ssl_socket peer{io, ssl_context};
      cout << "async_accept()" << endl;
      co_await sock.async_accept(peer.lowest_layer());
      if (!sock.is_open()) {
        peer.lowest_layer().close();
        break;
      }
      auto client = std::make_shared<Client>(this, std::move(peer));
      clients.insert(client);
      co_spawn(
          io, client->run() || client->timeout(),
          [&, client](std::exception_ptr e, auto) { clients.erase(client); });
    }
  } catch (const system_error& e) {
    if (e.code().value() != asio::error::operation_aborted) {
      cerr << "Error accepting: " << e.what() << " (" << e.what() << endl;
      throw;
    }
  } catch (const std::exception& e) {
    cerr << "Different kind of std::exception: "
         << boost::core::demangle(typeid(e).name()) << endl;
    throw;
  }
}

void Server::on_signal(err, int sig) {
  cout << "Received signal " << sig << endl;
  shutdown();
}

void Server::shutdown() noexcept {
  if (is_shutdown) {
    return;
  }
  is_shutdown = true;
  cout << "Shutting down" << endl;
  try {
    sock.close();
  } catch (...) {
  }

  std::ranges::for_each(clients, [](shared_ptr<Client> ptr) { ptr->close(); });

  clients.clear();
}