#ifndef SERVER_HPP
#define SERVER_HPP

#include <set>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/core/noncopyable.hpp>

class Server;

#include "net-types.hpp"
#include "client.hpp"

class Server : boost::noncopyable {
 public:
  explicit Server(ssl::context&&);

  void run();

 private:
  bool is_shutdown;
  io_context io;
  ssl::context ssl_context;
  std::set<std::shared_ptr<Client>> clients;
  acceptor sock;

  awaitable<void> do_run(const tcp::endpoint);
  void client_finished(std::shared_ptr<Client> client);
  void on_signal(err ec, int sig);
  void shutdown() noexcept;
};

#endif