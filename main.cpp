#include "server.hpp"
#include "net-types.hpp"

#include <filesystem>

int main(int argc, char *argv[]) {
  try {
    io_context io{};
    ssl::context ssl_context(ssl::context::tlsv13_server);
    ssl_context.use_certificate_file("certs/cert.pem", ssl::context::pem);
    ssl_context.use_private_key_file("certs/privkey.pem", ssl::context::pem);

    Server server{std::move(ssl_context)};
    server.run();
  } catch(std::exception &e) {
    std::cerr << e.what() << endl;
    return 1;
  }

  return 0;
}

