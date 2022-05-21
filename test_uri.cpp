#include <source_location>

#include "uri.hpp"

template <typename Os>
auto& operator<<(Os& os, std::source_location loc) {
  return os << loc.file_name() << ':' << loc.function_name() << ':'
            << loc.line() << ':' << loc.column();
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

template <>
struct expecter<bool> {
  bool got;
  std::source_location loc;

  ~expecter() {
    if (!got) {
      cerr << loc << ": Expected: true Got: false\n";
      std::exit(1);
    }
  }
};

template <typename T>
auto expect(T got, std::source_location loc = std::source_location::current()) {
  return expecter{got, loc};
}

void test_uri_view() {
  using data_t = struct { string_view scheme, host, port, path, query; };

  auto test_one = [](string_view uri, data_t data,
                     std::source_location loc =
                         std::source_location::current()) {
    UriView u{uri};
    expect(u.scheme(), loc) == data.scheme;
    expect(u.host(), loc) == data.host;
    expect(u.port(), loc) == data.port;
    expect(u.path(), loc) == data.path;
    expect(u.query(), loc) == data.query;
  };

  test_one("gemini://example.com:1966/foo/bar?asdf=zxcv&hjkl=vbnm", {
    scheme : "gemini",
    host : "example.com",
    port : "1966",
    path : "/foo/bar",
    query : "asdf=zxcv&hjkl=vbnm"
  });
  test_one("gemini://[::1]:1966/foo/bar?asdf=zxcv&hjkl=vbnm", {
    scheme : "gemini",
    host : "[::1]",
    port : "1966",
    path : "/foo/bar",
    query : "asdf=zxcv&hjkl=vbnm"
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

int main() { test_uri_view(); }