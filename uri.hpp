#ifndef URI_HPP
#define URI_HPP

#include <filesystem>
#include <map>
#include <system_error>
#include <variant>

#include "types.hpp"

namespace url {

/**
 * decodes '%' references in-place. It invalidates iterators obtained
 * from s. It returns std::errc::invalid_argument if s contains malformed %
 * sequences. The contents of s are undefined on error; make a copy if this is
 * an issue.
 */
std::errc decode(string &);

/**
 * @brief Encodes non-URL codepoints as % sequences.
 * 
 * @return std::errc 
 */
string encode(string_view);

/**
 * decode(s) decodes '%' references in s. It returns a pair of
 * std::errc and std::string. std::errc is std::errc::invalid_argument if s
 * contains malformed % sequences. The contents of the string are undefined on
 * error.
 */
std::variant<string, std::errc> decode(string_view);

namespace detail {
struct parser;
}

/**
 * Uri segments a URI into its component parts. It owns
 * memory and so is always valid.
 */
class Uri {
 public:
  using path_t = std::filesystem::path;
  using query_t = std::multimap<std::string, std::string>;

  // Parse a URI
  Uri(string_view);

  /// Parse a URI and resolve against a base URI
  Uri(string_view, const Uri &);

  string_view scheme() const noexcept, host() const noexcept,
      port() const noexcept, fragment() const noexcept;
  const path_t &path() const noexcept;
  const query_t &query() const noexcept;

  friend std::strong_ordering operator<=>(const Uri &, const Uri &);
  friend std::basic_ostream<char> &operator<<(std::basic_ostream<char> &,
                                              const Uri &);

  operator string() const;

 private:
  string _scheme;
  string _host;
  string _port;
  path_t _path;
  query_t _query;
  string _fragment;

  // Internal constructor
  Uri(detail::parser, const Uri *);
};

}  // namespace url
#endif