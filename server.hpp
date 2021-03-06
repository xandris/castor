#pragma once

class Server;

#include <boost/core/noncopyable.hpp>
#include <filesystem>
#include <functional>
#include <set>

#include "client.hpp"
#include "handler.hpp"
#include "net-types.hpp"
#include "response.hpp"

class Server : boost::noncopyable {
 public:
  explicit Server(ssl::context&&,
                  const std::map<std::filesystem::path, Handler>&);

  void run();
  std::optional<pair<std::reference_wrapper<Handler>, std::filesystem::path>>
  handler_for(const std::filesystem::path& p);

 private:
  bool is_shutdown{};
  io_context io{};
  std::set<asio::cancellation_signal*> clients{};
  ssl::context ssl_context;
  std::map<std::filesystem::path, Handler> handlers;
  acceptor sock;

  awaitable<void> do_run(const tcp::endpoint);
  void client_finished(std::shared_ptr<Client> client);
  void on_signal(err ec, int sig);
  void shutdown() noexcept;
};