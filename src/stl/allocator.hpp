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

template<typename Alloc, typename T>
struct rebind_alloc {
  using type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
};

template<typename Alloc, typename T>
using rebind_alloc_t = typename rebind_alloc<Alloc, T>::type;

template<typename Alloc, typename T>
concept standard_allocator_type = requires(Alloc alloc, Alloc alloc2,
                                           T* ptr, size_t n) {
  { alloc.allocate(n) } -> std::convertible_to<T*>;
  { alloc.deallocate(ptr, n) } -> std::same_as<void>;
  { alloc == alloc2 } -> std::convertible_to<bool>;
  { alloc != alloc2 } -> std::convertible_to<bool>;
} && std::same_as<typename std::allocator_traits<Alloc>::value_type, T>;

template<typename Alloc, typename T>
concept stateless_allocator_type = requires(T* ptr, size_t n) {
  { Alloc{}.allocate(n) } -> std::convertible_to<T*>;
  { Alloc{}.deallocate(ptr, n) } -> std::same_as<void>;
} && std::is_empty_v<Alloc> && std::same_as<typename std::allocator_traits<Alloc>::value_type, T>;

template<typename Alloc, typename T>
concept allocator_type = stateless_allocator_type<Alloc, T> || standard_allocator_type<Alloc, T>;

template<typename Pool>
concept mem_pool_type = requires(Pool& pool,
                                 void* ptr, size_t n, size_t szof, size_t align) {
  { pool.allocate(n*szof, align) } -> std::same_as<void*>;
  { pool.deallocate(ptr, n*szof) } -> std::same_as<void>;
};

template<typename Pool>
concept stateless_mem_pool_type = requires(void* ptr, size_t n, size_t szof, size_t align) {
  { Pool{}.allocate(n*szof, align) } -> std::same_as<void*>;
  { Pool{}.deallocate(ptr, n*szof) } -> std::same_as<void>;
} && std::is_empty_v<Pool>;

template<typename T, mem_pool_type Pool>
class alloc_adaptor {
public:
  using pool_type = Pool;
  using value_type = T;
  using pointer = T*;

public:
  alloc_adaptor(pool_type& pool) noexcept :
    _pool{pool} {}

  template<typename U>
  alloc_adaptor(const alloc_adaptor<U, Pool>& other) noexcept :
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

template<typename T, stateless_mem_pool_type Pool>
class stateless_alloc_adaptor {
public:
  using pool_type = Pool;
  using value_type = T;
  using pointer = T*;

public:
  stateless_alloc_adaptor() noexcept {}

  template<typename U>
  stateless_alloc_adaptor(const stateless_alloc_adaptor<U, Pool>&) noexcept {}

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
requires(
  stateless_allocator_type<Alloc, T> ||
  standard_allocator_type<Alloc, T>
)
class allocator_delete {
public:
  template<typename U>
  using rebind = allocator_delete<U, rebind_alloc_t<Alloc, U>>;

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
  allocator_delete(const allocator_delete<U, rebind_alloc_t<Alloc, U>>& other)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _alloc{other._alloc} {}

public:
  template<typename U = T>
  requires(std::convertible_to<U*, T*>)
  void operator()(U* ptr) noexcept(std::is_nothrow_destructible_v<T>) {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      static_cast<T*>(ptr)->~T();
    }
    _alloc.deallocate(ptr, 1);
  }

  template<typename U = T>
  requires(std::convertible_to<U*, T*>)
  void operator()(T* ptr, size_t n) noexcept(std::is_nothrow_destructible_v<T>) {
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

template<typename T, typename Alloc>
requires(
  stateless_allocator_type<Alloc, T>
)
class allocator_delete<T, Alloc> {
public:
  template<typename U>
  using rebind = allocator_delete<U, rebind_alloc_t<Alloc, U>>;

public:
  allocator_delete() noexcept {}

  allocator_delete(Alloc&&) noexcept {}

  allocator_delete(const Alloc&) noexcept {}

  template<typename U>
  allocator_delete(const allocator_delete<U, rebind_alloc_t<Alloc, U>>&) noexcept {}

public:
  template<typename U = T>
  requires(std::convertible_to<U*, T*>)
  void operator()(U* ptr) noexcept(std::is_nothrow_destructible_v<T>) {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      static_cast<T*>(ptr)->~T();
    }
    Alloc{}.deallocate(ptr, 1);
  }

  template<typename U = T>
  requires(std::convertible_to<U*, T*>)
  void operator()(T* ptr, size_t n) noexcept(std::is_nothrow_destructible_v<T>) {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (size_t i = 0; i < n; ++i) {
        static_cast<T*>(ptr+i)->~T();
      }
    }
    Alloc{}.deallocate(ptr, n);
  }

  const Alloc& get_allocator() const { return {}; }
};

template<typename Deleter, typename T>
struct rebind_deleter : public rebind_first_arg<Deleter, T> {};

template<typename Deleter, typename T>
requires requires() { typename Deleter::template rebind<T>; }
struct rebind_deleter<Deleter, T> {
  using type = typename Deleter::template rebind<T>;
};

template<typename Deleter, typename T>
using rebind_deleter_t = rebind_deleter<Deleter, T>::type;

template<typename T, typename AllocT>
using allocator_delete_alloc = allocator_delete<T, rebind_alloc_t<AllocT, T>>;

} // namespace ntf
