#ifndef URI_HPP
#define URI_HPP

#include "types.hpp"

// Forward declarations here.
class UriView;
class Uri;

/*
 * UriView segments a URI into its component parts. It doesn't own memory, it
 * just views into an existing buffer, which must remain valid. (See Uri for an
 * owning version.) It also doesn't normalize the URL since it doesn't own the
 * memory.
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
class UriView {
 public:
  UriView(string_view uri);
  string_view scheme() const,
  host() const,
  port() const,
  path() const,
  query() const;

  // Assemble a new URI string from parts
  operator string() const;

  // Compare with another UriView
  friend auto operator<=>(const UriView& a, const UriView& b) = default;

  friend std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os,
                                              const UriView&);

 protected:
  string_view _scheme, _host, _port, _path, _query;
};

/*
 * Uri segments a URI into its component parts. Unlike UriView, it owns
 * memory, and so is always valid. It can decode its parts because it
 * owns the memory.
 */
class Uri : public UriView {
 public:
  // Parse a string, copying its storage.
  explicit Uri(const string&);
  // Parse a string, taking its storage.
  explicit Uri(string&&);
  // Assemble a new Uri string from existing parts.
  Uri(const UriView&);
  // Copy an existing Uri, pointing UriView at the new storage.
  Uri(const Uri&);
  // Copy an existing Uri, taking its storage.
  Uri(Uri&&) = default;

  // Get a copy of this Uri's storage.
  // TODO: This may not be a valid URL because the parts have been decoded.
  string str() const;
  const string& ref() const;

  // Compare with another Uri or UriView
  // bool operator==(const Uri&) const;
  // bool operator==(const UriView&) const;
  friend std::strong_ordering operator<=>(const Uri&, const Uri&);

  friend std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os,
                                              const Uri&);

  // Replace this Uri by copying the contents of another one.
  Uri& operator=(const Uri&);
  // Replace this Uri by taking the storage of another one.
  Uri& operator=(Uri&&) = default;
  // Get a copy of this Uri's storage.
  operator string() const;
  // Get a reference to this Uri's storage.
  operator const string&() const;
  // Get a view of this Uri's storage.
  operator string_view() const;

 private:
  void normalize();
  string s;
};

#endif