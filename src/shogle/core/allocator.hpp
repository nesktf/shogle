#pragma once

#include <shogle/shogle.hpp>

namespace ntf {

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
  [[nodiscard]] pointer allocate(std::size_t n) {
    return reinterpret_cast<pointer>(_pool.allocate(n*sizeof(T), alignof(T)));
  }
  
  void deallocate(pointer ptr, std::size_t n) {
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
  [[nodiscard]] pointer allocate(std::size_t n) {
    return reinterpret_cast<pointer>(pool_type{}.allocate(n*sizeof(T), alignof(T)));
  }

  void deallocate(pointer ptr, std::size_t n) {
    pool_type{}.deallocate(ptr, n);
  }

public:
  constexpr bool operator==(const allocator_adaptor& rhs) const noexcept {
    return true;
  }
  constexpr bool operator!=(const allocator_adaptor& rhs) const noexcept {
    return !(*this == rhs);
  }
};

namespace impl {

template<typename P>
class basic_memory_arena {
public:
  template<typename T>
  using bind_adaptor = allocator_adaptor<T, P>;

private:
  struct arena_block {
    arena_block* next;
    std::size_t size;
    void* data;
  };

  static constexpr std::size_t MIN_BLOCK_SIZE = 4096;

public:
  basic_memory_arena() = default;
  basic_memory_arena(std::size_t initial_block_size) noexcept { init(initial_block_size); }

public:
  void init(std::size_t size) noexcept;

  void deallocate(void*, std::size_t) noexcept {}

  void clear(bool reallocate = false) noexcept;

public:
  template<typename T>
  allocator_adaptor<T, P> make_adaptor();

protected:
  arena_block* _insert_block(std::size_t size) noexcept;
  void _clear_pages() noexcept;

protected:
  arena_block* _block{nullptr};
  std::size_t _block_count{0}, _block_offset{0};

  std::size_t _used{0}, _allocated{0};

public:
  ~basic_memory_arena() noexcept { _clear_pages(); }
  NTF_DISABLE_COPY(basic_memory_arena);
};

} // namespace impl


class memory_arena : public impl::basic_memory_arena<memory_arena> {
public:
  using basic_memory_arena<memory_arena>::basic_memory_arena;

public:
  [[nodiscard]] void* allocate(std::size_t size, std::size_t align) noexcept;
};


class memory_stack : public impl::basic_memory_arena<memory_stack> {
private:
  static constexpr std::size_t DEFAULT_ALLOC_LIMIT = 4096;

public:
  memory_stack() = default;
  memory_stack(std::size_t alloc_limit) noexcept :
    basic_memory_arena(alloc_limit), _alloc_limit(alloc_limit) {}

public:
  [[nodiscard]] void* allocate(std::size_t size, std::size_t align) noexcept;
  void reset() noexcept;

private:
  std::size_t _alloc_limit{DEFAULT_ALLOC_LIMIT};
};

} // namespace ntf

#ifndef NTF_ALLOCATOR_INL
#include <shogle/core/allocator.inl>
#endif
