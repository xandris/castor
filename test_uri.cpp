#include <source_location>

#include "uri.hpp"

std::ostream& operator<<(std::ostream& os, std::source_location loc) {
  return os << loc.file_name() << ':' << loc.function_name() << ':'
            << loc.line() << ':' << loc.column();
}

std::ostream& operator<<(std::ostream& os, std::errc ec) {
  return os << std::make_error_code(ec).message();
}

std::ostream& operator<<(std::ostream& os, url::Uri::query_t q) {
  os << '{';
  for (auto& [k, v] : q) {
    os << " {" << std::quoted(k) << ": " << std::quoted(v) << "} ";
  }
  return os << '}';
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, const pair<T, U>& ec) {
  return os << '(' << ec.first << ", " << ec.second;
}

template <typename T>
concept Streamable = requires(T t, std::ostream& os) {
  { os << t } -> std::same_as<std::ostream&>;
};

template <Streamable T>
struct expecter {
  T got;
  std::source_location loc;

  template <Streamable U>
  void operator==(U expected) {
    if (got != expected) {
      cerr << loc << " Expected: " << expected << " Got: " << got << '\n';
      std::exit(1);
    }
  }
};

template <Streamable T>
auto expect(T got, std::source_location loc = std::source_location::current()) {
  return expecter<T>{got, loc};
}

using data_t = struct {
  string_view scheme, host, port, path;
  url::Uri::query_t query;
  string_view fragment;
};

void test_uri() {
  auto test_one = [](string_view uri, data_t data,
                     std::source_location loc =
                         std::source_location::current()) {
    url::Uri u{uri};
    expect(u.scheme(), loc) == data.scheme;
    expect(u.host(), loc) == data.host;
    expect(u.port(), loc) == data.port;
    expect(u.path(), loc) == data.path;
    expect(u.query(), loc) == data.query;
    expect(u.fragment(), loc) == data.fragment;
  };

  test_one("gemini://example.com:1966/foo/bar?asdf=zxcv&hjkl=vbnm#xyzzy", {
    scheme : "gemini",
    host : "example.com",
    port : "1966",
    path : "/foo/bar",
    query : {{"asdf", "zxcv"}, {"hjkl", "vbnm"}},
    fragment : "xyzzy",
  });
  test_one("gemini://example.com:1966/foo/bar?asdf=z%26&hjkl=vbnm#xyzzy", {
    scheme : "gemini",
    host : "example.com",
    port : "1966",
    path : "/foo/bar",
    query : {{"asdf", "z&"}, {"hjkl", "vbnm"}},
    fragment : "xyzzy",
  });
  test_one("gemini://[::1]:1966/foo/bar?asdf=zxcv&hjkl=vbnm#xyzzy", {
    scheme : "gemini",
    host : "[::1]",
    port : "1966",
    path : "/foo/bar",
    query : {{"asdf", "zxcv"}, {"hjkl", "vbnm"}},
    fragment : "xyzzy"
  });
  test_one("foo/bar", {path : "foo/bar"});
  test_one("//asdf:123:", {host : "asdf:123"});
  test_one("//asdf/foo/bar", {host : "asdf", path : "/foo/bar"});
  test_one("gemini:foo/bar", {scheme : "gemini", path : "foo/bar"});
  test_one("gemini:", {scheme : "gemini"});
  test_one(":::::", {path : "::::"});
  test_one("", {});
  test_one("/////", {path : "///"});
  test_one("foo:?asdf=zxcv", {scheme : "foo", query : {{"asdf", "zxcv"}}});
  test_one("foo:/?asdf=zxcv",
           {scheme : "foo", path : "/", query : {{"asdf", "zxcv"}}});
}

void test_base_url() {
  url::Uri base{"gemini://example.com:8080/asdf/zxcv?foo=bar#frag"};

  auto test_one = [&](string_view uri, data_t data,
                      std::source_location loc =
                          std::source_location::current()) {
    url::Uri u{uri, base};
    expect(u.scheme(), loc) == data.scheme;
    expect(u.host(), loc) == data.host;
    expect(u.port(), loc) == data.port;
    expect(u.path(), loc) == data.path;
    expect(u.query(), loc) == data.query;
    expect(u.fragment(), loc) == data.fragment;
  };

  test_one("", {
    scheme : "gemini",
    host : "example.com",
    port : "8080",
    path : "/asdf/zxcv",
    query : {{"foo", "bar"}},
    fragment : "frag"
  });

  test_one("#frag2", {
    scheme : "gemini",
    host : "example.com",
    port : "8080",
    path : "/asdf/zxcv",
    query : {{"foo", "bar"}},
    fragment : "frag2"
  });

  test_one("?asdf=zxcv", {
    scheme : "gemini",
    host : "example.com",
    port : "8080",
    path : "/asdf/zxcv",
    query : {{"asdf", "zxcv"}},
  });

  test_one("/newroot", {
    scheme : "gemini",
    host : "example.com",
    port : "8080",
    path : "/newroot",
  });

  test_one("newfile", {
    scheme : "gemini",
    host : "example.com",
    port : "8080",
    path : "/asdf/newfile",
  });

  test_one("//newhost.example.com:1234/newroot", {
    scheme : "gemini",
    host : "newhost.example.com",
    port : "1234",
    path : "/newroot",
  });

  test_one("https://newhost.example.com:1234/newroot", {
    scheme : "https",
    host : "newhost.example.com",
    port : "1234",
    path : "/newroot",
  });
}

void test_encode() {
  for (auto&& in : std::initializer_list<string_view>{
           "gemini://example.com/p1/p2?asdf=zxcv&qwer=hjkl#frag",
           "gemini://example.com/p1/p2?asdf=zx%26cv&qwer=hjkl#frag",
           "gemini://example.com/p1/p2?asdf=zx%0fcv&qwer=hjkl#frag",
           "gemini:",
           "//example.com/p1/p2",
           "?asdf=zxcv",
           "#frag",
       }) {
    expect(string(url::Uri{in})) == in;
  }
}

void test_url_decode() {
  for (auto&& [in, out] : std::initializer_list<pair<string_view, string_view>>{
           {"", ""},
           {"%2e", "."},
           {"%2E", "."},
           {"%2E%2e%2E", "..."},
           {"asdf%2E%2f%2Ezxcv", "asdf./.zxcv"}}) {
    auto dec = url::decode(in);
    expect(dec.index()) == 0;
    expect(std::get<0>(dec)) == out;
  }

  for (auto&& in : std::initializer_list<string_view>{
           "%",
           "asdf%",
           "asdf%2e%",
           "asdf%2e%2",
       }) {
    auto dec = url::decode(in);
    expect(dec.index()) == 1;
    expect(std::get<1>(dec)) == std::errc::invalid_argument;
  }
}

int main() {
  test_url_decode();
  test_uri();
  test_encode();
  test_base_url();
}