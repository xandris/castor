#include <openssl/ssl.h>

#include <type_traits>
#include <utility>

namespace openssl {

namespace detail {
template <typename Ptr, int(UpRef)(Ptr), void(Free)(Ptr)>
requires std::is_pointer_v<Ptr>
class Refcnt {
 public:
  using mytype = Refcnt<Ptr, UpRef, Free>;

  Refcnt() = delete;
  Refcnt(Ptr p) : p{p} {
    if (p) UpRef(p);
  }
  Refcnt(const mytype& o) : Refcnt{o.p} {}
  Refcnt(mytype&& o) : p{std::move(o.p)} {}
  ~Refcnt() {
    if (p) Free(std::move(p));
  }

  Ptr operator->() { return p; }
  Ptr operator*() { return p; }

 private:
  Ptr p;
};

}  // namespace detail

/// Reference-counted pointer to an OpenSSL session (SSL_SESSION*).
/// Always increments the ref count on initialization. How to borrow?
class Session {
 public:
  using ptr_t =
      detail::Refcnt<SSL_SESSION*, SSL_SESSION_up_ref, SSL_SESSION_free>;

  template<typename T>
  Session(T&& t) : p{std::move(t)} {}

  //// True if this wraps a non-null handle.
  operator bool() { return *p; }

  /** Get the servername from the SNI extension.
   *
   * @par
   * Returns a pointer to a C string containing the TLS server
   * name if set, or nullptr otherwise.
   * 
   * @remarks This only works on TLS 1.2 and below.
   */
  const char* hostname() { return SSL_SESSION_get0_hostname(*p); }

 private:
  ptr_t p;
};

/** Reference-counted pointer to an OpenSSL connection (SSL*).
 *
 * @par
 * Always increments the ref count on initialization. How to borrow?
 */
class Ssl {
 public:
  using ptr_t = detail::Refcnt<SSL*, SSL_up_ref, SSL_free>;

  template<typename T>
  Ssl(T&& t) : p{std::move(t)} {}

  //// True if this wraps a non-null handle.
  operator bool() { return *p; }

  /** Get the servername from the SNI extension.
   *
   * @par
   * Returns a pointer to a C string containing the TLS server
   * name if set, or nullptr otherwise.
   */
  const char* servername() {
    auto type{SSL_get_servername_type(*p)};
    return type < 0 ? nullptr : SSL_get_servername(*p, type);
  }

  /**
   * @brief gets a shared reference to this connection's session.
   *
   * session() returns a pointer to a C string containing the TLS server
   * name if set, or nullptr otherwise.
   */
  Session session() { return Session{SSL_get_session(*p)}; }

 private:
  ptr_t p;
};

}  // namespace openssl