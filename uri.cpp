#include "uri.hpp"

#include <cassert>
#include <cctype>
#include <charconv>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace url {

std::errc decode(string &s) {
  string::size_type left{s.find('%', left)};
  if (left == string::npos)
    // No valid % sequences
    return std::errc();

  string_view in{&s[left], s.length() - left};

  for (auto out{&s[left]};;) {
    if (in.length() < 3)
      // Invalid; truncated % at end
      return std::errc::invalid_argument;

    uint8_t val;

    auto res{std::from_chars(&in[1], &in[3], val, 16)};
    if (res.ptr != &in[3]) {
      return std::errc::invalid_argument;
    }

    in.remove_prefix(3);

    *out = val;
    ++out;

    auto right{in.find('%')};
    if (right == string_view::npos) {
      // No more '%' escapes
      in.copy(out, in.length());
      out += in.length();
      s.erase(string::iterator{out}, s.end());
      if (s.capacity() >> 1 > s.length()) s.shrink_to_fit();
      return std::errc();
    }
    in.copy(out, right);
    out += right;
    in.remove_prefix(right);
  }
}

std::variant<string, std::errc> decode(string_view s) {
  string t{s};
  auto ec{decode(t)};
  if (ec != std::errc{}) return ec;
  return t;
}

namespace detail {

/**
 * parser segments a URI into its component parts. It doesn't own memory,
 * it just views into an existing buffer, which must remain valid. It also
 * doesn't normalize the URL since it doesn't own the memory.
 *
 * For example, this URL:
 *
 * gemini://example.com:1966/foo/.././bar?asdf=zxcv&hjkl=vbnm
 *
 * Would result in this UriView, where the parts in parentheses are skipped
 * over:
 *
 * - Scheme: "gemini" (":")
 *
 * - Host:   ("//") "example.com"
 *
 * - Port:   (":") "1966"
 *
 * - Path:   "/foo/.././bar"?asdf=zxcv#bookmark"
 *
 * - Query:  ("?") ("asdf=zxcv&hjkl=vbnm")
 *
 * ("user[:pass]@host" and "#fragment" syntaxes not supported yet.)
 *
 * Components are string_views. Missing parts are default-constructed string
 * views. Given this URL:
 *
 * asdf/zxcv
 *
 * Only Path is non-empty.
 */
struct parser {
  string_view scheme, host, port, path, query, fragment;

  parser(string_view);
};

parser::parser(string_view uri) {
  // [scheme:[//host[:port]]/]path[?query][#fragment]
  // If there is a colon before the first slash, question mark, or pound,
  // treat it as the scheme delimiter.
  auto right = uri.find_first_of(":/?#");
  if (right != string_view::npos && uri[right] == ':') {
    // Consume the scheme
    scheme = uri.substr(0, right);
    // This is right+1 because the delimiter : belongs to the scheme.
    uri.remove_prefix(right + 1);
  }

  // [//host[:port]/]path[?query][#fragment]
  if (uri.substr(0, 2) == "//") {
    uri.remove_prefix(2);
    // Consume the host
    right = uri.find_first_of("/?#");
    if (right == string_view::npos) {
      right = uri.length();
    }
    host = uri.substr(0, right);

    // This isn't right+1 because the delimiter doesn't belong to the host.
    uri.remove_prefix(right);

    right = host.find_last_of(":]");
    if (right != string_view::npos && host[right] == ':') {
      // host[right] == ']' means IPv6 address, no port
      // could be malformed address, idk
      // Otherwise, split on the :
      port = host.substr(right + 1);
      host = host.substr(0, right);
    }
  }

  // path[?query][#fragment]
  right = uri.find_first_of("?#");
  if (right == string_view::npos) right = uri.length();

  // Consume the path
  path = uri.substr(0, right);
  uri.remove_prefix(right);

  if (uri.length() > 0 && uri[0] == '?') {
    uri.remove_prefix(1);
    right = uri.find("#");
    if (right == string_view::npos) right = uri.length();
    query = uri.substr(0, right);
    uri.remove_prefix(right);
  }

  if (uri.length() > 0 && uri[0] == '#') {
    fragment = uri.substr(1);
  }
}

std::variant<Uri::query_t, std::errc> parse_query(string_view s) {
  Uri::query_t ret;
  string k, v;
  std::errc ec;

  for (string_view::size_type right{0}; !s.empty();
       s.remove_prefix(right + (right != s.length()))) {
    right = s.find('&');
    if (right == string_view::npos) {
      right = s.length();
    }
    auto eq = s.substr(0, right).find('=');
    if (eq == string_view::npos) {
      k = s.substr(0, right);
      ec = decode(k);
      v = "";
    } else {
      k = s.substr(0, eq);
      v = s.substr(eq + 1, right - eq - 1);
      ec = decode(k);
      if (ec == std::errc{}) {
        ec = decode(v);
      }
    }

    if (ec != std::errc{}) {
      return ec;
    }

    ret.insert({k, v});
  }

  return ret;
}

const string_view urlchars{
    "!$&'()*+,-./"
    "0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ_~abcdefghijklmnopqrstuvwxyz"};
const string_view path_urlchars{
    "!$&'()*+,-./"
    "0123456789:;=@ABCDEFGHIJKLMNOPQRSTUVWXYZ_~abcdefghijklmnopqrstuvwxyz"};
const string_view query_urlchars{
    "!$'()*+,-./"
    "0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ_~abcdefghijklmnopqrstuvwxyz"};

struct urlencoder {
  string_view s;
  string_view allowed{urlchars};
};

std::ostream &operator<<(std::ostream &os, const urlencoder &u) {
  for (auto s = u.s; !s.empty();) {
    auto right = s.find_first_not_of(u.allowed);
    if (right == string_view::npos) {
      os << s;
      break;
    }
    os << s.substr(0, right) << '%' << std::setw(2) << std::setfill('0') << std::hex << int{s[right]};
    s.remove_prefix(right + 1);
  }
  return os;
}

}  // namespace detail

string encode(string_view s) {
  std::ostringstream os;
  os << detail::urlencoder{s};
  return os.str();
}

/*
 * URI IMPLEMENTATION
 */
Uri::Uri(string_view s) : Uri{detail::parser{s}, nullptr} {}
Uri::Uri(string_view s, const Uri &base) : Uri{detail::parser{s}, &base} {}
Uri::Uri(detail::parser p, const Uri *base) {
  string decoded_path{p.path};
  if (decode(decoded_path) != std::errc()) {
    throw std::invalid_argument{"Invalid URL"};
  }
  string decoded_fragment{p.fragment};
  if (decode(decoded_fragment) != std::errc()) {
    throw std::invalid_argument{"Invalid URL"};
  }

  if (base) {
    if (!p.scheme.empty()) {
      goto scheme;
    }
    _scheme = base->_scheme;
    if (!(p.host.empty() && p.port.empty())) {
      goto host;
    }
    _host = base->_host;
    _port = base->_port;

    if (decoded_path.starts_with('/')) {
      goto path;
    }

    if (p.path.empty()) {
      _path = base->_path;
    } else {
      _path = base->_path;
      _path.replace_filename(decoded_path);
      goto query;
    }

    if (!p.query.empty()) {
      goto query;
    }
    _query = base->_query;

    if (!p.fragment.empty()) {
      goto fragment;
    }
    _fragment = base->_fragment;

    goto fixup;
  }

scheme:
  _scheme = p.scheme;
host:
  _host = p.host;
  _port = p.port;
path:
  _path = p.path;
query : {
  auto maybeQuery = detail::parse_query(p.query);
  if (maybeQuery.index() == 1) {
    throw std::invalid_argument{"Invalid URL"};
  } else {
    _query = std::move(std::get<0>(maybeQuery));
  }
}
fragment:
  _fragment = p.fragment;
fixup:
  _path = _path.lexically_normal();
}

string_view Uri::scheme() const noexcept { return _scheme; }
string_view Uri::host() const noexcept { return _host; }
string_view Uri::port() const noexcept { return _port; }
const Uri::path_t &Uri::path() const noexcept { return _path; }
const Uri::query_t &Uri::query() const noexcept { return _query; }
string_view Uri::fragment() const noexcept { return _fragment; }

Uri::operator string() const {
  std::ostringstream os;
  os << *this;
  return os.str();
}

std::ostream &operator<<(std::ostream &os, const Uri &u) {
  if (!u.scheme().empty()) {
    os << detail::urlencoder{u.scheme()} << ':';
  }
  if (!(u.host().empty() && u.port().empty())) {
    os << "//" << detail::urlencoder{u.host()};
    if (!u.port().empty()) {
      os << ':' << detail::urlencoder{u.port()};
    }
    if (!u.path().empty() && u.path().is_relative()) {
      os << '/';
    }
  }
  os << detail::urlencoder{u.path().native(), detail::path_urlchars};
  if (!u.query().empty()) {
    os << '?';
    auto first{true};

    for (auto &[k, v] : u.query()) {
      if (!first) {
        os << '&';
      }
      first = false;

      os << detail::urlencoder{k, detail::query_urlchars};
      if (!v.empty()) {
        os << '=' << detail::urlencoder{v, detail::query_urlchars};
      }
    }
  }
  if (!u.fragment().empty()) {
    os << '#' << detail::urlencoder{u.fragment()};
  }
  return os;
}

}  // namespace url