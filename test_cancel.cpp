#include "net-types.hpp"

using namespace asio::experimental::awaitable_operators;

struct noisy {
  string_view name;
  ~noisy() { cerr << '~' << name << '\n'; }
};

awaitable<void> job() {
  noisy n{"subjob"};
  timer t{co_await asio::this_coro::executor, 3s};
  auto e=t.get_executor();
  cout<<e.target_type().name()<<'\n';
  co_await t.async_wait();

  cout << "job done\n";
}

awaitable<void> canceller(asio::cancellation_signal& sig) {
  noisy n{"canceller"};
  timer t{co_await asio::this_coro::executor, 1s};
  co_await t.async_wait();
  sig.emit(asio::cancellation_type_t::terminal);
}

int main() {
  noisy n{"main"};
  io_context io;

  cout << "main: spawning\n";
  asio::cancellation_signal sig;
  co_spawn(io, job(), asio::bind_cancellation_slot(sig.slot(), [](std::exception_ptr e) {
    if (e) {
      try {
        std::rethrow_exception(e);
      } catch (const std::exception &e) {
        cerr << "Error: " << e.what() << '\n';
      }
    } else {
      cout << "main done\n";
    }
  }));

  co_spawn(io, canceller(sig), asio::detached);

  io.run();
}