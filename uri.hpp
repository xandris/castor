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
  string_view scheme() const;
  string_view host() const;
  string_view port() const;
  string_view path() const;
  string_view query() const;

 private:
  string_view _scheme, _host, _port, _path, _query;
};

/*
 * Uri segments a URI into its component parts. Unlike UriView, it owns
 * memory, and so is always valid.
 */
class Uri : UriView {};

#endif