#include <source_location>

#include "uri.hpp"

template<typename Os>
auto& operator<<(Os &os, std::source_location loc) {
  return os << loc.file_name() << ':' << loc.function_name() << ':' << loc.line() << ':' << loc.column();
}

template <typename T>
struct expecter {
  T got;
  std::source_location loc;

  template <typename U>
  void operator==(U expected) {
    if(got != expected) {
      cerr << loc << " Expected: " << expected << " Got: " << got << '\n';
      std::exit(1);
    }
  }
};

template<>
struct expecter<bool> {
  bool got;
  std::source_location loc;

  ~expecter() {
    if(!got) {
      cerr << loc << ": Expected: true Got: false\n";
      std::exit(1);
    }
  }
};

template <typename T>
auto expect(T got, std::source_location loc = std::source_location::current()) {
  return expecter{got, loc};
}


int main() {
  UriView u{"gemini://example.com:1966/foo/bar?asdf=zxcv&hjkl=vbnm"};

  expect(u.scheme()) == "gemini";
  expect(u.host()) == "example.com";
  expect(u.port()) == "1966";
  expect(u.path()) == "/foo/bar";
  expect(u.query()) == "asdf=zxcv&hjkl=vbnm";

  u = UriView{"gemini://[::1]:1966/foo/bar?asdf=zxcv&hjkl=vbnm"};

  expect(u.scheme()) == "gemini";
  expect(u.host()) == "[::1]";
  expect(u.port()) == "1966";
  expect(u.path()) == "/foo/bar";
  expect(u.query()) == "asdf=zxcv&hjkl=vbnm";

  u = UriView{"foo/bar"};
  expect(u.scheme()) == "";
  expect(u.host()) == "";
  expect(u.port()) == "";
  expect(u.path()) == "foo/bar";
  expect(u.query()) == "";

  u = UriView{"//asdf:123:"};
  expect(u.scheme()) == "";
  expect(u.host()) == "asdf:123";
  expect(u.port()) == "";
  expect(u.path()) == "";
  expect(u.query()) == "";

  u = UriView{"//asdf/foo/bar"};
  expect(u.scheme()) == "";
  expect(u.host()) == "asdf";
  expect(u.port()) == "";
  expect(u.path()) == "/foo/bar";
  expect(u.query()) == "";

  u = UriView{"gemini:foo/bar"};
  expect(u.scheme()) == "gemini";
  expect(u.host()) == "";
  expect(u.path()) == "foo/bar";

  u = UriView{"gemini:"};
  expect(u.scheme()) == "gemini";
  expect(u.host()) == "";
  expect(u.port()) == "";
  expect(u.path()) == "";
  expect(u.query()) == "";

  u = UriView{":::::"};
  expect(u.scheme()) == "";
  expect(u.host()) == "";
  expect(u.port()) == "";
  expect(u.path()) == "::::";
  expect(u.query()) == "";

  u = UriView{""};
  expect(u.scheme()) == "";
  expect(u.host()) == "";
  expect(u.port()) == "";
  expect(u.path()) == "";
  expect(u.query()) == "";

  u = UriView{"/////"};
  expect(u.scheme()) == "";
  expect(u.host()) == "";
  expect(u.port()) == "";
  expect(u.path()) == "///";
  expect(u.query()) == "";

  u = UriView{"foo:?asdf=zxcv"};
  expect(u.scheme()) == "foo";
  expect(u.host()) == "";
  expect(u.port()) == "";
  expect(u.path()) == "";
  expect(u.query()) == "asdf=zxcv";

  u = UriView{"foo:/?asdf=zxcv"};
  expect(u.scheme()) == "foo";
  expect(u.host()) == "";
  expect(u.port()) == "";
  expect(u.path()) == "/";
  expect(u.query()) == "asdf=zxcv";
}