#include "uri.hpp"

#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>

/*
 * URIVIEW IMPLEMENTATION
 */

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

UriView::operator string() const {
  // Try precompute the size; the length of each part plus 1-2 character
  // separators where needed.
  string::size_type len = _scheme.length() + _host.length() + _port.length() +
                          _path.length() + _query.length() + !_scheme.empty() +
                          !_host.empty() * 2 + !_port.empty() + !_query.empty();
  string ret{};
  ret.reserve(len);

  if (!_scheme.empty()) {
    ret += ':';
    ret += _scheme;
  }

  if (!_host.empty() || !_port.empty()) {
    ret.append(2, '/');
    ret += _host;
    if (!_port.empty()) ret += ':';
    ret += _port;
    if (!_path.empty() && !_path.starts_with('/')) {
      ret += '/';
    }
  }

  ret += _path;

  if (!_query.empty()) {
    ret += '?';
    ret += _query;
  }

  return ret;
}

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os,
                                     const UriView& u) {
  if (!u._scheme.empty()) os << u._scheme << ':';
  if (!(u._host.empty() && u._port.empty())) {
    os << "//" << u._host;
    if (!u._port.empty()) os << ':' << u._port;
    if (!u._path.empty() || !u._path.starts_with('/')) {
      os << '/';
    }
  }
  os << u._path;
  if (!u._query.empty()) {
    os << '?' << u._query;
  }
  return os;
}

/*
 * URI IMPLEMENTATION
 */

Uri::Uri(const string& s) : s{s}, UriView{s} {}
Uri::Uri(string&& s) : s{s}, UriView{s} {}
Uri::Uri(const UriView& v) : s{v}, UriView{s} {}
Uri::Uri(const Uri& u) : s{u.s}, UriView{s} {}

string Uri::str() const { return s; }
const string& Uri::ref() const { return s; }

// bool Uri::operator==(const Uri& other) const {
// return UriView(*this) == UriView(other);
//}
// bool Uri::operator==(const UriView& other) const {
// return UriView(*this) == other;
//}

std::strong_ordering operator<=>(const Uri& a, const Uri& b) {
  return UriView(a) <=> UriView(b);
}

Uri& Uri::operator=(const Uri& other) {
  if (this == &other) return *this;
  s = other.s;
  UriView(*this) = string_view{s};
  return *this;
}

Uri::operator string() const { return str(); }
Uri::operator const string&() const { return ref(); }
Uri::operator string_view() const { return s; }

void Uri::normalize() {
  if (_path.length() == 0)
    // Empty path is already normal
    return;

  // The _path segment must lie within s
  assert(s.data() <= _path.data());
  assert(_path.data() <= s.data() + s.length());

  // The un-examined path data
  auto p{_path};

  if (p[0] == '/') {
    // Skip a leading '/'
    p.remove_prefix(1);
  } else
    // Skip over leading '..' path segments
    while (p.starts_with("../")) {
      p.remove_prefix(3);
    }

  // Should .. pop a path segment?
  // Only if there are non .. segments to the left.
  // This records how many non .. segments there are to the left.
  size_t depth{0};

  // Skip over segments that aren't '', '.', or '..'
  while (!p.empty()) {
    auto right = p.find('/');
    if (right == string_view::npos) right = p.length();
    switch (right) {
      case 2:
        if (p[1] != '.') break;
        // fall-through
      case 1:
        if (p[0] != '.') break;
        // fall-through
      case 0:
        goto ffwd;
    }
    ++depth;
    p.remove_prefix(right + (right != p.length()));
  }

  // Already normalized.
  return;

ffwd:

  // Where to write the path next segment
  auto out{&s[p.data() - s.data()]};

  while (!p.empty()) {
    // Stride forward to the next slash
    auto right = p.find('/');
    // (possibly empty) path segment from left..right
    if (right == string::npos)
      right = p.length();
    else
      right += 1;
    // right points at the end of the string or just after the slash
    // 012345678    012345     01234
    // asdf/zxcv    asdf/      asdf
    //      ^            ^         ^
    if (right == 3 && p[0] == '.' && p[1] == '.') {
      if (depth > 0) {
        --depth;
        out = &s[s.rfind('/', out - s.data() - 2) + 1];
      }
    } else if ((right == 2 && p[0] == '.') || right == 0) {
      // Just consume
    } else {
      // Copy "asdf/" (not at end) or "asdf" (at end)
      p.copy(out, right);
      out += right;
    }
    p.remove_prefix(right);
  }

  s.resize(out-s.data());
}