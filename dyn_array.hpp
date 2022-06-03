#include <memory>

template <typename T>
class dyn_array {
 public:
  using iterator_type = T*;
  using const_iterator_type = const T*;
  using reference_type = T&;
  using const_reference_type = T&&;
  dyn_array(size_t _sz) : sz{_sz}, array{new T[sz]} {}
  dyn_array(const dyn_array& o) : sz{o.sz}, array{new T[sz]} {
    std::copy(o.begin(), o.end(), begin());
  }
  dyn_array(dyn_array&& o) { std::swap(*this, o); }
  iterator_type begin() noexcept { return array.get(); }
  iterator_type end() noexcept { return begin() + sz; }
  const_iterator_type begin() const noexcept { return array.get(); }
  const_iterator_type end() const noexcept { return begin() + sz; }
  reference_type operator[](size_t n) noexcept { return begin()[n]; }
  const_reference_type operator[](size_t n) const noexcept {
    return begin()[n];
  }
  constexpr size_t length() const noexcept { return sz; }

 private:
  size_t sz;
  std::unique_ptr<T[]> array;
};