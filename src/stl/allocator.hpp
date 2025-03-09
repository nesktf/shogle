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

template<typename Alloc, typename T>
concept standard_allocator_type = requires(Alloc alloc, Alloc alloc2,
                                           T* ptr, size_t n) {
  { alloc.allocate(n) } -> std::convertible_to<T*>;
  { alloc.deallocate(ptr, n) } -> std::same_as<void>;
  { alloc == alloc2 } -> std::convertible_to<bool>;
  { alloc != alloc2 } -> std::convertible_to<bool>;
};

template<typename Pool, typename T>
concept mem_pool_type = requires(Pool& pool,
                                 void* ptr, size_t n) {
  { pool.allocate(n*sizeof(T), alignof(T)) } -> std::same_as<void*>;
  { pool.deallocate(ptr, n*sizeof(T)) } -> std::same_as<void>;
};

template<typename Pool, typename T>
concept stateless_mem_pool_type = requires(void* ptr, size_t n) {
  { Pool{}.allocate(n*sizeof(T), alignof(T)) } -> std::same_as<void*>;
  { Pool{}.deallocate(ptr, n*sizeof(T)) } -> std::same_as<void>;
};

template<typename T, mem_pool_type<T> Pool>
class alloc_adaptor {
public:
  using pool_type = Pool;
  using value_type = T;
  using pointer = T*;

public:
  alloc_adaptor(pool_type& pool) noexcept :
    _pool{pool} {}

  template<typename U>
  alloc_adaptor(alloc_adaptor<U, Pool>& other) noexcept :
    _pool{other._pool} {}

  template<typename U>
  alloc_adaptor(alloc_adaptor<U, Pool>&& other) noexcept :
    _pool{other._pool} {}

public:
  [[nodiscard]] pointer allocate(size_t n) {
    return reinterpret_cast<pointer>(_pool.allocate(n*sizeof(T), alignof(T)));
  }
  
  void deallocate(pointer ptr, size_t n) {
    _pool.deallocate(ptr, n*sizeof(T));
  }

public:
  constexpr bool operator==(const alloc_adaptor& rhs) const noexcept {
    if constexpr (has_operator_equals<T>) {
      return _pool == rhs._pool;
    } else {
      return (std::addressof(_pool) == std::addressof(rhs._pool));
    }
  }
  constexpr bool operator!=(const alloc_adaptor& rhs) const noexcept {
    if constexpr (has_operator_nequals<T>) {
      return _pool != rhs._pool;
    } else {
      return !(*this == rhs);
    }
  }

private:
  pool_type& _pool;
};

template<typename T, stateless_mem_pool_type<T> Pool>
class stateless_alloc_adaptor {
public:
  using pool_type = Pool;
  using value_type = T;
  using pointer = T*;

public:
  stateless_alloc_adaptor() noexcept {}

  template<typename U>
  stateless_alloc_adaptor(stateless_alloc_adaptor<U, Pool>&) noexcept {}

  template<typename U>
  stateless_alloc_adaptor(stateless_alloc_adaptor<U, Pool>&&) noexcept {}

public:
  [[nodiscard]] pointer allocate(size_t n) {
    return reinterpret_cast<pointer>(pool_type{}.allocate(n*sizeof(T), alignof(T)));
  }

  void deallocate(pointer ptr, size_t n) {
    pool_type{}.deallocate(ptr, n*sizeof(T));
  }

public:
  constexpr bool operator==(const stateless_alloc_adaptor&) const noexcept {
    return true;
  }
  constexpr bool operator!=(const stateless_alloc_adaptor& rhs) const noexcept {
    return !(*this == rhs);
  }
};

template<typename T, typename Alloc>
requires(standard_allocator_type<Alloc, std::remove_pointer_t<std::decay_t<T>>>)
class allocator_delete {
public:
  allocator_delete()
  noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
    _alloc{} {}

  allocator_delete(Alloc&& alloc)
  noexcept(std::is_nothrow_move_constructible_v<Alloc>) :
    _alloc{std::move(alloc)} {}

  allocator_delete(const Alloc& alloc)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>):
    _alloc{alloc} {}

  template<typename U>
  requires(std::convertible_to<T*, U*>)
  allocator_delete(const allocator_delete<U, Alloc>& other)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _alloc{other._alloc} {}

public:
  void operator()(T* ptr) noexcept(std::is_nothrow_destructible_v<T>) {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      ptr->~T();
    }
    _alloc.deallocate(ptr, 1);
  }

  const Alloc& get_allocator() const { return _alloc; }

private:
  Alloc _alloc;
};

template<typename T, typename Alloc>
class allocator_delete<T[], Alloc> {
public:
  allocator_delete()
  noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
    _alloc{} {}

  allocator_delete(Alloc&& alloc_)
  noexcept(std::is_nothrow_move_constructible_v<Alloc>) :
    _alloc{std::move(alloc_)} {}

  allocator_delete(const Alloc& alloc_)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>):
    _alloc{alloc_} {}

  template<typename U>
  requires(std::convertible_to<T(*)[], U(*)[]>)
  allocator_delete(const allocator_delete<U[], Alloc>& other)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _alloc{other._alloc} {}

public:
  template<typename U = T>
  requires(std::convertible_to<T(*)[], U(*)[]>)
  void operator()(U* ptr, size_t n) noexcept(std::is_nothrow_destructible_v<T>) {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (size_t i = 0; i < n; ++i) {
        static_cast<T*>(ptr+i)->~T();
      }
    }
    _alloc.deallocate(ptr, n);
  }

  const Alloc& get_allocator() const { return _alloc; }

private:
  Alloc _alloc;
};

template<typename T>
class allocator_delete<T, std::allocator<T>> {
public:
  allocator_delete() noexcept = default;

  allocator_delete(std::allocator<T>&&) noexcept {}

  allocator_delete(const std::allocator<T>&) noexcept {}

  template<typename U>
  requires(std::convertible_to<T*, U*>)
  allocator_delete(const allocator_delete<U, std::allocator<U>>&) noexcept {}

public:
  void operator()(T* ptr) const noexcept(std::is_nothrow_destructible_v<T>) {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      ptr->~T();
    }
    std::allocator<T>{}.deallocate(ptr, 1);
  }

  const std::allocator<T>& get_allocator() const { return std::allocator<T>{}; }
};

template<typename T>
class allocator_delete<T[], std::allocator<T>> {
public:
  allocator_delete() noexcept = default;

  allocator_delete(std::allocator<T>&&) noexcept {}

  allocator_delete(const std::allocator<T>&) noexcept {}

  template<typename U>
  requires(std::convertible_to<T(*)[], U(*)[]>)
  allocator_delete(const allocator_delete<U[], std::allocator<U>>&) noexcept {}

public:
  template<typename U = T>
  requires(std::convertible_to<T(*)[], U(*)[]>)
  void operator()(U* ptr, size_t n) const noexcept(std::is_nothrow_destructible_v<T>) {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (size_t i = 0; i < n; ++i) {
        static_cast<T*>(ptr+i)->~T();
      }
    }
    std::allocator<T>{}.deallocate(ptr, n);
  }

  const std::allocator<T>& get_allocator() const { return std::allocator<T>{}; }
};

template<typename T, typename Pool>
class allocator_delete<T, stateless_alloc_adaptor<T, Pool>> {
public:
  allocator_delete() noexcept = default;

  allocator_delete(stateless_alloc_adaptor<T, Pool>&&) noexcept {}

  allocator_delete(const stateless_alloc_adaptor<T, Pool>&) noexcept {}

  template<typename U>
  requires(std::convertible_to<T*, U*>)
  allocator_delete(const allocator_delete<U, stateless_alloc_adaptor<U, Pool>>&) noexcept {}

public:
  void operator()(T* ptr) const noexcept(std::is_nothrow_destructible_v<T>) {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      ptr->~T();
    }
    stateless_alloc_adaptor<T, Pool>{}.deallocate(ptr, 1);
  }

  const stateless_alloc_adaptor<T, Pool>& get_allocator() const { return {}; }
};

template<typename T, typename Pool>
class allocator_delete<T[], stateless_alloc_adaptor<T, Pool>> {
public:
  allocator_delete() noexcept = default;

  allocator_delete(stateless_alloc_adaptor<T, Pool>&&) noexcept {}

  allocator_delete(const stateless_alloc_adaptor<T, Pool>&) noexcept {}

  template<typename U>
  requires(std::convertible_to<T*, U*>)
  allocator_delete(const allocator_delete<U, stateless_alloc_adaptor<U, Pool>>&) noexcept {}

public:
  template<typename U = T>
  requires(std::convertible_to<T(*)[], U(*)[]>)
  void operator()(U* ptr, size_t n) const noexcept(std::is_nothrow_destructible_v<T>) {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (size_t i = 0; i < n; ++i) {
        static_cast<T*>(ptr+i)->~T();
      }
    }
    stateless_alloc_adaptor<T, Pool>{}.deallocate(ptr, n);
  }

  const stateless_alloc_adaptor<T, Pool>& get_allocator() const { return {}; }
};

} // namespace ntf
