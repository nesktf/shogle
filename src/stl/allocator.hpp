#pragma once

#include "./types.hpp"

namespace ntf {

namespace impl {

inline size_t align_fw_adjust(void* ptr, size_t align) noexcept {
  uintptr_t iptr = reinterpret_cast<uintptr_t>(ptr);
  // return ((iptr - 1u + align) & -align) - iptr;
  return align - (iptr & (align - 1u));
}

inline void* ptr_add(void* p, uintptr_t sz) noexcept {
  return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + sz);
}

} // namespace impl


template<typename T, typename Allocator>
struct rebind_alloc {
  using type = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
};

template<typename T, typename Allocator>
using rebind_alloc_t = typename rebind_alloc<T, Allocator>::type;


template<typename P>
struct memory_pool_traits {
  static constexpr bool is_stateless = false;
};


template<typename T, typename P,
  bool = memory_pool_traits<P>::is_stateless>
class allocator_adaptor {
public:
  using pool_type = P;
  using value_type = T;
  using pointer = T*;

public:
  allocator_adaptor(pool_type& pool) :
    _pool(pool) {}

  template<typename U>
  allocator_adaptor(allocator_adaptor<U, P>& other) :
    _pool(other._pool) {}

public:
  [[nodiscard]] pointer allocate(size_t n) {
    return reinterpret_cast<pointer>(_pool.allocate(n*sizeof(T), alignof(T)));
  }
  
  void deallocate(pointer ptr, size_t n) {
    _pool.deallocate(ptr, n);
  }

public:
  constexpr bool operator==(const allocator_adaptor& rhs) const noexcept {
    if constexpr (has_operator_equals<T>) {
      return _pool == rhs._pool;
    } else {
      return (std::addressof(_pool) == std::addressof(rhs._pool));
    }
  }
  constexpr bool operator!=(const allocator_adaptor& rhs) const noexcept {
    if constexpr (has_operator_nequals<T>) {
      return _pool != rhs._pool;
    } else {
      return !(*this == rhs);
    }
  }

private:
  pool_type& _pool;
};

template<typename T, typename P>
class allocator_adaptor<T, P, true> {
public:
  using pool_type = P;
  using value_type = T;
  using pointer = T*;

public:
  allocator_adaptor() = default;

  template<typename U>
  allocator_adaptor(allocator_adaptor<U, P>&) {}

public:
  [[nodiscard]] pointer allocate(size_t n) {
    return reinterpret_cast<pointer>(pool_type{}.allocate(n*sizeof(T), alignof(T)));
  }

  void deallocate(pointer ptr, size_t n) {
    pool_type{}.deallocate(ptr, n);
  }

public:
  constexpr bool operator==(const allocator_adaptor&) const noexcept {
    return true;
  }
  constexpr bool operator!=(const allocator_adaptor& rhs) const noexcept {
    return !(*this == rhs);
  }
};

template<typename Alloc, typename T>
concept standard_allocator_type = requires(Alloc alloc, Alloc alloc2,
                                           T* ptr, size_t n) {
  { alloc.allocate(n) } -> std::convertible_to<T*>;
  { alloc.deallocate(ptr, n) } -> std::same_as<void>;
  { alloc == alloc2 } -> std::convertible_to<bool>;
  { alloc != alloc2 } -> std::convertible_to<bool>;
};

} // namespace ntf
