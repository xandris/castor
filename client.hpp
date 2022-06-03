#pragma once

class Client;

#include "net-types.hpp"
#include "server.hpp"

#include <boost/core/noncopyable.hpp>

class Client : boost::noncopyable {
 public:
  Client(Server &, ssl_socket &&);

  awaitable<void> run();
  awaitable<void> timeout();
  void close();

 private:
  Server &server;
  ssl_socket peer;
  timer _timeout;
};