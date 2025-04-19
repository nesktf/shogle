#pragma once

#include "./types.hpp"

namespace ntf {

constexpr uint64 kibs(uint64 count) { return count << 10; }
constexpr uint64 mibs(uint64 count) { return count << 20; }
constexpr uint64 gibs(uint64 count) { return count << 30; }
constexpr uint64 tibs(uint64 count) { return count << 40; }

constexpr size_t align_fw_adjust(void* ptr, size_t align) noexcept {
  uintptr_t iptr = std::bit_cast<uintptr_t>(ptr);
  // return ((iptr - 1u + align) & -align) - iptr;
  return align - (iptr & (align - 1u));
}

constexpr void* ptr_add(void* p, uintptr_t sz) noexcept {
  return std::bit_cast<void*>(std::bit_cast<uintptr_t>(p)+sz);
}

template<typename Alloc, typename T>
struct rebind_alloc {
  using type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
};

template<typename Alloc, typename T>
using rebind_alloc_t = typename rebind_alloc<Alloc, T>::type;

template<typename Alloc, typename T>
concept allocator_type = requires(Alloc& alloc, Alloc& alloc2,
                                  T* ptr, size_t n) {
  { alloc.allocate(n) } -> std::convertible_to<T*>;
  { alloc.deallocate(ptr, n) } -> std::same_as<void>;
  // { alloc == alloc2 } -> std::convertible_to<bool>;
  // { alloc != alloc2 } -> std::convertible_to<bool>;
} && std::same_as<typename std::allocator_traits<Alloc>::value_type, T>;

template<typename Pool>
concept mem_pool_type = requires(Pool& pool,
                                 void* ptr, size_t n, size_t szof, size_t align) {
  { pool.allocate(n*szof, align) } -> std::same_as<void*>;
  { pool.deallocate(ptr, n*szof) } -> std::same_as<void>;
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

namespace impl {

template<typename T, typename PoolT>
struct mem_pool_alloc_store {
  constexpr mem_pool_alloc_store(PoolT& pool) noexcept :
    _pool{&pool} {}

  constexpr T* _allocate(size_t n) {
    return reinterpret_cast<T*>(_pool->allocate(n*sizeof(T), alignof(T)));
  }

  constexpr void _deallocate(T* ptr, size_t n) {
    _pool->deallocate(ptr, n*sizeof(T));
  }

  constexpr PoolT& _get_pool() noexcept { return *_pool; }

  PoolT* _pool;
};

template<typename T, typename PoolT>
requires(std::is_empty_v<PoolT> && std::is_default_constructible_v<PoolT>)
struct mem_pool_alloc_store<T, PoolT> : private PoolT {
  constexpr mem_pool_alloc_store() noexcept {}

  constexpr mem_pool_alloc_store(PoolT&) noexcept :
    PoolT{} {}

  constexpr T* _allocate(size_t n) {
    return reinterpret_cast<T*>(PoolT::allocate(n*sizeof(T), alignof(T)));
  }

  constexpr void _deallocate(T* ptr, size_t n) {
    PoolT::deallocate(ptr, n*sizeof(T));
  }

  constexpr PoolT& _get_pool() { return static_cast<PoolT&>(*this); }
};

} // namespace impl

template<typename T, mem_pool_type PoolT>
class allocator_adaptor : public impl::mem_pool_alloc_store<T, PoolT> {
public:
  using pool_type = PoolT;
  using value_type = T;
  using pointer = T*;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  template<typename U>
  using rebind = allocator_adaptor<U, PoolT>;

private:
  using store_base = impl::mem_pool_alloc_store<T, PoolT>;

public:
  constexpr allocator_adaptor() noexcept :
    store_base{} {}

  constexpr explicit allocator_adaptor(PoolT& pool) noexcept :
    store_base{pool} {}

  template<typename U>
  constexpr allocator_adaptor(const allocator_adaptor<U, PoolT>& other) noexcept :
    store_base{other.get_pool()} {}

public:
  constexpr pointer allocate(size_t n) {
    return store_base::_allocate(n);
  }

  constexpr void deallocate(pointer ptr, size_t n) {
    store_base::_deallocate(ptr, n);
  }

public:
  constexpr PoolT& get_pool() { return store_base::_get_pool(); }

public:
  constexpr bool operator==(const allocator_adaptor& other) const noexcept {
    if constexpr (has_operator_equals<PoolT>) {
      return get_pool() == other.get_pool();
    } else {
      return &get_pool() == &other.get_pool();
    }
  }

  constexpr bool operator!=(const allocator_adaptor& other) const noexcept {
    if constexpr (has_operator_equals<PoolT>) {
      return get_pool() != other.get_pool();
    } else {
      return !(*this == other);
    }
}
};

template<typename T>
class virtual_allocator {
public:
  using value_type = T;
  using pointer = T*;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  static constexpr size_t BUFFER_SIZE = 2*sizeof(void*);

  template<typename U>
  using rebind = virtual_allocator<U>;

private:
  struct vtable_t {
    T* (*allocate)(void*, size_t);
    void (*deallocate)(void*, T*, size_t);
    void (*destroy)(void*);
    void (*copy)(void*, const void*);
  };

  template<typename Alloc>
  static constexpr vtable_t vtable_for {
    .allocate = +[](void* alloc, size_t n) {
      return static_cast<Alloc*>(alloc)->allocate(n);
    },
    .deallocate = +[](void* alloc, T* ptr, size_t n) {
      static_cast<Alloc*>(alloc)->deallocate(ptr, n);
    },
    .destroy = +[](void* alloc) {
      // TODO: Maybe only accept trivially destructible allocators for an optimization?
      static_assert(std::is_destructible_v<Alloc>, "Alloc has to be destructible");
      static_assert(std::is_nothrow_destructible_v<Alloc>, "Alloc has to be nothrow destructible");
      static_cast<Alloc*>(alloc)->~Alloc();
    },
    .copy = +[](void* alloc, const void* other) {
      // TODO: Maybe only accept trivially copyable allocators for an optimization?
      static_assert(std::copy_constructible<Alloc>, "Alloc has to be copy constructible");
      std::construct_at(static_cast<Alloc*>(alloc), *static_cast<const Alloc*>(other));
    },
  };

public:
  template<typename Alloc>
  requires(allocator_type<std::remove_cvref_t<Alloc>, T> && sizeof(Alloc) <= BUFFER_SIZE)
  virtual_allocator(Alloc&& alloc)
  noexcept(is_nothrow_forward_constructible<Alloc>) :
    _vtable{&vtable_for<std::remove_cvref_t<Alloc>>}
  {
    _construct<std::remove_cvref_t<Alloc>>(std::forward<Alloc>(alloc));
  }

  template<typename Alloc, typename... Args>
  requires(allocator_type<std::remove_cvref_t<Alloc>, T> && sizeof(Alloc) <= BUFFER_SIZE)
  virtual_allocator(std::in_place_type_t<Alloc>, Args&&... args)
  noexcept(std::is_nothrow_constructible_v<Alloc, Args...>) :
    _vtable{&vtable_for<std::remove_cvref_t<Alloc>>}
  {
    _construct<std::remove_cvref_t<Alloc>>(std::forward<Args>(args)...);
  }

  template<typename Alloc, typename U, typename... Args>
  requires(allocator_type<std::remove_cvref_t<Alloc>, T> && sizeof(Alloc) <= BUFFER_SIZE)
  virtual_allocator(std::in_place_type_t<Alloc>, std::initializer_list<U> il, Args&&... args)
  noexcept(std::is_nothrow_constructible_v<Alloc, Args...>) :
    _vtable{&vtable_for<std::remove_cvref_t<Alloc>>}
  {
    _construct<std::remove_cvref_t<Alloc>>(il, std::forward<Args>(args)...);
  }

  virtual_allocator(const virtual_allocator& other) :
    _vtable{other._vtable}
  {
    _copy(&other._buffer[0]);
  }

  virtual_allocator(virtual_allocator&& other) noexcept :
    _vtable{std::move(other._vtable)}
  {
    _move(&other._buffer[0]);
    other._vtable = nullptr;
  }

  ~virtual_allocator() noexcept { _destroy(); }

public:
  pointer allocate(size_t n) {
    return std::invoke(_vtable->allocate,
                       reinterpret_cast<void*>(&_buffer[0]), n);
  }

  void deallocate(pointer ptr, size_t n) {
    return _vtable->deallocate(reinterpret_cast<void*>(&_buffer[0]), ptr, n);
  }

public:
  virtual_allocator& operator=(const virtual_allocator& other) {
    _destroy();

    _vtable = other._vtable;
    _copy(&other._buffer[0]);

    return *this;
  }

  virtual_allocator& operator=(virtual_allocator&& other) noexcept {
    _destroy();
    
    _vtable = std::move(other._vtable);
    _move(&other._buffer[0]);

    other._vtable = nullptr;

    return *this;
  }

private:
  template<typename Alloc, typename... Args>
  void _construct(Args&&... args) {
    std::construct_at(reinterpret_cast<Alloc*>(&_buffer[0]), std::forward<Args>(args)...);
  }

  void _copy(const uint8* other_buf) {
    std::invoke(_vtable->copy,
                reinterpret_cast<void*>(&_buffer[0]), reinterpret_cast<const void*>(other_buf));
  }

  void _move(uint8* other_buf) {
    std::memcpy(&_buffer[0], other_buf, BUFFER_SIZE);
  }

  void _destroy() noexcept {
    if (_vtable) {
      std::invoke(_vtable->destroy, reinterpret_cast<void*>(&_buffer[0]));
    }
  }

private:
  uint8 _buffer[BUFFER_SIZE];
  const vtable_t* _vtable;
};

namespace impl {

template<typename T, typename Alloc>
struct alloc_del_dealloc : private Alloc {
  alloc_del_dealloc() :
    Alloc{} {}

  alloc_del_dealloc(const Alloc& alloc) :
    Alloc{alloc} {}

  alloc_del_dealloc(Alloc&& alloc) :
    Alloc{std::move(alloc)} {}

  void _dealloc(T* ptr, size_t sz) {
    Alloc::deallocate(ptr, sz);
  }

  Alloc& _get_allocator() { return static_cast<Alloc&>(*this); }
};

} // namespace impl

template<typename T, allocator_type<T> Alloc>
class allocator_delete : private impl::alloc_del_dealloc<T, Alloc> {
public:
  template<typename U>
  using rebind = allocator_delete<U, rebind_alloc_t<Alloc, U>>;

private:
  using deall_base = impl::alloc_del_dealloc<T, Alloc>;

public:
  allocator_delete()
  noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
    deall_base{} {}

  allocator_delete(Alloc&& alloc)
  noexcept(std::is_nothrow_move_constructible_v<Alloc>) :
    deall_base{std::move(alloc)} {}

  allocator_delete(const Alloc& alloc)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>):
    deall_base{alloc} {}

  template<typename U>
  allocator_delete(const allocator_delete<U, rebind_alloc_t<Alloc, U>>& other)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    deall_base{other.get_allocator()} {}

public:
  template<typename U = T>
  requires(std::convertible_to<T*, U*>)
  void operator()(U* ptr)
  noexcept(std::is_nothrow_destructible_v<T>)
  {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      static_cast<T*>(ptr)->~T();
    }
    deall_base::_dealloc(ptr, 1u);
  }

  template<typename U = T>
  requires(std::convertible_to<T*, U*>)
  void operator()(U* ptr, size_t n)
  noexcept(std::is_nothrow_destructible_v<T>)
  {
    static_assert(is_complete<T>, "Cannot destroy incomplete type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (U* it = ptr; it != ptr+n; ++it) {
        static_cast<T*>(it)->~T();
      }
    }
    deall_base::_dealloc(ptr, n);
  }

  Alloc& get_allocator() noexcept { return deall_base::_get_allocator(); }
  const Alloc& get_allocator() const noexcept { return deall_base::_get_allocator(); }
};

template<typename T>
using default_alloc_del = allocator_delete<T, std::allocator<T>>;

template<typename T>
using virtual_alloc_del = allocator_delete<T, virtual_allocator<T>>;

} // namespace ntf
