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

pair<string, std::errc> decode(const string &s) {
  string t{s};
  auto ec{decode(t)};
  return {t, ec};
}

/*
 * Uri::_parser segments a URI into its component parts. It doesn't own memory,
 * it just views into an existing buffer, which must remain valid. (See Uri for
 * an owning version.) It also doesn't normalize the URL since it doesn't own
 * the memory.
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
struct Uri::_parser {
  string_view scheme, host, port, path, query, fragment;

  _parser(string_view);
};

Uri::_parser::_parser(string_view uri) {
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

/*
 * URI IMPLEMENTATION
 */
Uri::Uri(string_view s) : Uri{Uri::_parser{s}} {}
Uri::Uri(Uri::_parser p)
    : _scheme{p.scheme}, _host{p.host}, _port{p.port}, _query{} {
  string dec{p.path};
  if (decode(dec) != std::errc()) {
    throw std::invalid_argument{"Invalid URL"};
  }
  _path = path_t{std::move(dec)}.lexically_normal();

  dec = p.fragment;
  if (decode(dec) != std::errc()) {
    throw std::invalid_argument{"Invalid URL"};
  }
  _fragment = std::move(dec);
}

string_view Uri::scheme() const { return _scheme; }
string_view Uri::host() const { return _host; }
string_view Uri::port() const { return _port; }
const Uri::path_t &Uri::path() const { return _path; }
const Uri::query_t &Uri::query() const { return _query; }
string_view Uri::fragment() const { return _fragment; }

}  // namespace url