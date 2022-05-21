#include "uri.hpp"

#include <stdexcept>

UriView::UriView(string_view uri) {
  // [scheme:[//host[:port]]/]path[?query]
  // If there is a colon before the first slash or question mark, we treat it as
  // the scheme delimiter.
  auto right = uri.find_first_of(":?/");
  if (right != string_view::npos && uri[right] == ':') {
    // Consume the scheme
    _scheme = uri.substr(0, right);
    // This is right+1 because the delimiter : belongs to the scheme.
    uri.remove_prefix(right + 1);
  }

  // [//host[:port]/]path[?query]
  if (uri.substr(0, 2) == "//") {
    uri.remove_prefix(2);
    // Consume the host
    right = uri.find_first_of("?/");
    if (right == string_view::npos) {
      right = uri.length();
    }
    auto host = uri.substr(0, right);

    // This isn't right+1 because the delimiter doesn't belong to the host.
    uri.remove_prefix(right);

    right = host.find_last_of(":]");
    if (right != string_view::npos) {
      if (host[right] == ']') {
        // IPv6 address, no port
        // could be malformed address, idk
        _host = host;
      } else {
        // hostname:port
        _host = host.substr(0, right);
        _port = host.substr(right + 1);
      }
    } else {
      // No port
      _host = host;
    }
  }

  // path[?query]
  right = uri.find('?');
  if (right == string_view::npos) {
    // No query, just path
    _path = uri;
    return;
  } else {
    // Consume the path
    _path = uri.substr(0, right);
    uri.remove_prefix(right);
  }

  // ?query
  _query = uri.substr(1);
}

string_view UriView::scheme() const { return _scheme; }
string_view UriView::host() const { return _host; }
string_view UriView::port() const { return _port; }
string_view UriView::path() const { return _path; }
string_view UriView::query() const { return _query; }