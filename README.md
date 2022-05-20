# Castor Gemini Server

Goal: A Gemini protocol server in C++20 using [`boost::asio`][boost.asio] and
coroutines.

The last time I used C++ was over 20 years ago, so I'm hoping for a useful end
product and to learn modern C++.

I'm still getting started, so don't expect a functioning server yet!

## Very modest roadmap

- [x] Open a listening socket
- [x] Accept a connection
- [x] Do an SSL handshake on the socket
- [x] Read a line of input
- [ ] Clean up resources automatically
- [ ] Handle signals
- [ ] Parse a URL
- [ ] Define a request
- [ ] Define a response
  
  It would be nice to have behavior similar to Java's ServletResponse; send the
  header on the first write to the output stream.
  
- [ ] Define a handler
- [ ] Bind handlers to URLs
- [ ] Make a filesystem handler
- [ ] Define middleware
- [ ] Process middleware
- [ ] Set "acceptable CAs" for the handshake
- [ ] Get the client certificate from the handshake

[boost.asio]: https://www.boost.org/doc/libs/release/libs/asio/
