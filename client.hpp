#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/core/noncopyable.hpp>

class Client;

#include "net-types.hpp"
#include "server.hpp"

class Client : boost::noncopyable {
 public:
  Client(Server*, ssl_socket&&);

  awaitable<void> run();
  awaitable<void> timeout();
  void close();

 private:
  timer _timeout;
  ssl_socket peer;
  Server *server;
};

#endif