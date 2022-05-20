#ifndef NET_TYPES_HPP
#define NET_TYPES_HPP

#include "types.hpp"

#include <boost/asio.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/ssl.hpp>

namespace {
    namespace asio = boost::asio;
    namespace ssl = asio::ssl;
    using namespace asio::experimental::awaitable_operators;
    using asio::experimental::as_tuple;
    using asio::ip::tcp;
    using namespace std::literals::chrono_literals;

    using asio::experimental::as_tuple_t;
    using executor = asio::use_awaitable_t<asio::any_io_executor>::executor_with_default<asio::any_io_executor>;
    using timer = asio::steady_timer::rebind_executor<executor>::other;
    using acceptor = tcp::acceptor::rebind_executor<executor>::other;
    using socket = asio::basic_stream_socket<tcp, executor>;
    using ssl_socket = asio::ssl::stream<socket>;
    using asio::io_context;
    using asio::awaitable;
    using asio::async_read_until;
    using asio::co_spawn;
    using asio::detached;
}

#endif