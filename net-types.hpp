#ifndef NET_TYPES_HPP
#define NET_TYPES_HPP

#define BOOST_ASIO_NO_DEPRECATED
#define BOOST_ASIO_HAS_IO_URING

#include <boost/asio.hpp>
#include <boost/core/demangle.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#ifdef BOOST_ASIO_NO_DEPRECATED
#define saved_BOOST_ASIO_NO_DEPRECATED
#undef BOOST_ASIO_NO_DEPRECATED
#endif

#include <boost/asio/experimental/as_tuple.hpp>

#ifdef saved_BOOST_ASIO_NO_DEPRECATED
#define BOOST_ASIO_NO_DEPRECATED
#undef saved_BOOST_ASIO_NO_DEPRECATED
#endif

#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/ssl.hpp>

#include "types.hpp"

namespace {
using system_error = boost::system::system_error;
using err = boost::system::error_code;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
using namespace asio::experimental::awaitable_operators;
using asio::experimental::as_tuple;
using asio::ip::tcp;

using asio::experimental::as_tuple_t;
using executor = asio::use_awaitable_t<
    asio::any_io_executor>::executor_with_default<asio::any_io_executor>;
using timer = asio::steady_timer::rebind_executor<executor>::other;
using acceptor = tcp::acceptor::rebind_executor<executor>::other;
using socket = asio::basic_stream_socket<tcp, executor>;
using ssl_socket = asio::ssl::stream<socket>;
using asio::async_read_until;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::io_context;
} // namespace

#endif