#include <source_location>

#include "uri.hpp"

std::ostream& operator<<(std::ostream& os, std::source_location loc) {
  return os << loc.file_name() << ':' << loc.function_name() << ':'
            << loc.line() << ':' << loc.column();
}

std::ostream& operator<<(std::ostream& os, std::errc ec) {
  return os << std::make_error_code(ec).message();
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, const pair<T, U>& ec) {
  return os << '(' << ec.first << ", " << ec.second;
}

template <typename T>
struct expecter {
  T got;
  std::source_location loc;

  template <typename U>
  void operator==(U expected) {
    if (got != expected) {
      cerr << loc << " Expected: " << expected << " Got: " << got << '\n';
      std::exit(1);
    }
  }
};

template <typename T>
auto expect(T got, std::source_location loc = std::source_location::current()) {
  return expecter<T>{got, loc};
}

void test_uri() {
  using data_t = struct { string_view scheme, host, port, path, query, fragment; };

  auto test_one = [](string_view uri, data_t data,
                     std::source_location loc =
                         std::source_location::current()) {
    url::Uri u{uri};
    expect(u.scheme(), loc) == data.scheme;
    expect(u.host(), loc) == data.host;
    expect(u.port(), loc) == data.port;
    expect(u.path(), loc) == data.path;
    expect(u.fragment(), loc) == data.fragment;
  };

  test_one("gemini://example.com:1966/foo/bar?asdf=zxcv&hjkl=vbnm#xyzzy", {
    scheme : "gemini",
    host : "example.com",
    port : "1966",
    path : "/foo/bar",
    query : "asdf=zxcv&hjkl=vbnm",
    fragment: "xyzzy"
  });
  test_one("gemini://[::1]:1966/foo/bar?asdf=zxcv&hjkl=vbnm#xyzzy", {
    scheme : "gemini",
    host : "[::1]",
    port : "1966",
    path : "/foo/bar",
    query : "asdf=zxcv&hjkl=vbnm",
    fragment: "xyzzy"
  });
  test_one("foo/bar", {path : "foo/bar"});
  test_one("//asdf:123:", {host : "asdf:123"});
  test_one("//asdf/foo/bar", {host : "asdf", path : "/foo/bar"});
  test_one("gemini:foo/bar", {scheme : "gemini", path : "foo/bar"});
  test_one("gemini:", {scheme : "gemini"});
  test_one(":::::", {path : "::::"});
  test_one("", {});
  test_one("/////", {path : "///"});
  test_one("foo:?asdf=zxcv", {scheme : "foo", query : "asdf=zxcv"});
  test_one("foo:/?asdf=zxcv",
           {scheme : "foo", path : "/", query : "asdf=zxcv"});
}

void test_url_decode() {
  using good_case = struct{const string good, bad;};
  for (auto&& [in, out] :
       std::initializer_list<good_case>{{"", ""},
                                        {"%2e", "."},
                                        {"%2E", "."},
                                        {"%2E%2e%2E", "..."},
                                        {"asdf%2E%2f%2Ezxcv", "asdf./.zxcv"}}) {
    expect(url::decode(in)) == pair{out, std::errc()};
  }

  for (auto&& in : std::initializer_list<const string>{
           "%",
           "asdf%",
           "asdf%2e%",
           "asdf%2e%2",
       }) {
    expect(url::decode(in).second) == std::errc::invalid_argument;
  }
}

int main() { test_url_decode(); test_uri(); }