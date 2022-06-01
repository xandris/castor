#include <openssl/ssl.h>

#include <type_traits>
#include <utility>

namespace openssl {

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

class Session {
 public:
  using ptr_t = Refcnt<SSL_SESSION*, SSL_SESSION_up_ref, SSL_SESSION_free>;

  Session(ptr_t ptr) : p{ptr} {}
  operator bool() { return *p; }

  const char* hostname() { return SSL_SESSION_get0_hostname(*p); }

 private:
  ptr_t p;
};

class Ssl {
 public:
  using ptr_t = Refcnt<SSL*, SSL_up_ref, SSL_free>;
  Ssl(ptr_t ptr) : p{ptr} {}
  operator bool() { return *p; }

  const char* servername() {
    auto type{SSL_get_servername_type(*p)};
    return type < 0 ? nullptr : SSL_get_servername(*p, type);
  }
  Session session() { return Session{SSL_get_session(*p)}; }

 private:
  ptr_t p;
};

}  // namespace openssl