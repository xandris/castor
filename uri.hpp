#ifndef URI_HPP
#define URI_HPP

#include <filesystem>
#include <map>
#include <system_error>

#include "types.hpp"

namespace url {

/*
 * url_decode decodes '%' references in-place. It invalidates iterators obtained
 * from s. It returns std::errc::invalid_argument if s contains malformed %
 * sequences. The contents of s are undefined on error; make a copy if this is
 * an issue.
 */
std::errc decode(string &);

/*
 * url_decode decodes '%' references in the given string. It returns a pair of
 * std::errc and std::string. std::errc is std::errc::invalid_argument if s
 * contains malformed % sequences. The contents of the string are undefined on
 * error.
 */
pair<string, std::errc> decode(const string &);

/*
 * Uri segments a URI into its component parts. Unlike UriView, it owns
 * memory, and so is always valid. It can decode its parts because it
 * owns the memory.
 */
class Uri {
  using path_t = std::filesystem::path;
  using query_t = std::multimap<std::string, std::string>;

 public:
  // Parse a Uri
  Uri(string_view s);

  string_view scheme() const, host() const, port() const, fragment() const;
  const path_t &path() const;
  const query_t &query() const;
  friend std::strong_ordering operator<=>(const Uri &, const Uri &);

  friend std::basic_ostream<char> &operator<<(std::basic_ostream<char> &,
                                              const Uri &);

 private:
  string _scheme;
  string _host;
  string _port;
  path_t _path;
  query_t _query;
  string _fragment;

  struct _parser;

  // Internal constructor
  Uri(_parser p);
};

}  // namespace url
#endif