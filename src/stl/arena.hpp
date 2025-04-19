#pragma once

#include "./allocator.hpp"
#include "./expected.hpp"

namespace ntf {

class arena_block_manager {
public:
  template<typename T>
  using adaptor_type = allocator_adaptor<T, arena_block_manager>;

public:
  arena_block_manager(void* block, size_t block_sz) noexcept :
    _block{block},
    _used{0u}, _allocated{block_sz} {}

public:
  void clear() noexcept;

  void* allocate(size_t size, size_t alignment) noexcept;
  void deallocate(void*, size_t) noexcept {}

public:
  template<typename T>
  T* allocate(size_t n) {
    return static_cast<T*>(allocate(n*sizeof(T), alignof(T)));
  }

  template<typename T, typename... Args>
  T* construct(Args&&... args)
  noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    T* ptr = allocate<T>(1u);
    if (!ptr) {
      return ptr;
    }
    std::construct_at(ptr, std::forward<Args>(args)...);
    return ptr;
  }

  template<typename T>
  void destroy(T* ptr)
  noexcept(std::is_nothrow_destructible_v<T>)
  {
    ptr->~T();
    deallocate(ptr, 1u);
  }

public:
  size_t size() const noexcept { return _used; }
  size_t capacity() const noexcept { return _allocated; }
  void* data() { return _block; }

protected:
  void* _block;
  size_t _used;
  size_t _allocated;
};

class fixed_arena : public arena_block_manager {
public:
  template<typename T>
  using adaptor_type = allocator_adaptor<T, fixed_arena>;

private:
  fixed_arena(void* base, size_t sz) noexcept :
    arena_block_manager{base, sz} {}

public:
  static expected<fixed_arena, error<void>> from_size(size_t size) noexcept;

public:
  NTF_DECLARE_MOVE_ONLY(fixed_arena);
};

class linked_arena {
public:
  template<typename T>
  using adaptor_type = allocator_adaptor<T, linked_arena>;

private:
  linked_arena(void* block, size_t block_sz) noexcept :
    _block{block}, _block_used{0u},
    _total_used{0u}, _allocated{block_sz} {}

public:
  static expected<linked_arena, error<void>> from_size(size_t size) noexcept;

public:
  void* allocate(size_t size, size_t alignment) noexcept;
  void deallocate(void*, size_t) noexcept {}

public:
  template<typename T>
  T* allocate(size_t n) {
    return static_cast<T*>(allocate(n*sizeof(T), alignof(T)));
  }

  template<typename T, typename... Args>
  T* construct(Args&&... args)
  noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    T* ptr = allocate<T>(1u);
    if (!ptr) {
      return ptr;
    }
    std::construct_at(ptr, std::forward<Args>(args)...);
    return ptr;
  }

  template<typename T>
  void destroy(T* ptr)
  noexcept(std::is_nothrow_destructible_v<T>)
  {
    ptr->~T();
    deallocate(ptr, 1u);
  }

  void clear() noexcept;

public:
  size_t size() const noexcept { return _total_used; }
  size_t capacity() const noexcept { return _allocated; }

private:
  void* _block;
  size_t _block_used;
  size_t _total_used, _allocated;

public:
  NTF_DECLARE_MOVE_ONLY(linked_arena);
};

template<size_t buffer_sz>
requires(buffer_sz > 0u)
class static_arena {
public:
  template<typename T>
  using adaptor_type = allocator_adaptor<T, static_arena>;

  static constexpr size_t BUFFER_SIZE = buffer_sz;

public:
  constexpr static_arena() noexcept :
    _used{0u} {}

public:
  constexpr void* allocate(size_t size, size_t alignment) noexcept {
    const size_t available = BUFFER_SIZE-_used;
    const size_t padding = align_fw_adjust(ptr_add(data(), _used), alignment);
    const size_t required = padding+size;
    if (available < required) {
      return nullptr;
    }
    void* ptr = ptr_add(data(), _used+padding);
    _used += required;
    return ptr;
  }
  constexpr void deallocate(void*, size_t) noexcept {}

  constexpr void clear() noexcept { _used = 0u; }

public:
  template<typename T>
  constexpr T* allocate(size_t n) noexcept {
    return static_cast<T*>(allocate(n*sizeof(T), alignof(T)));
  }

  template<typename T, typename... Args>
  constexpr T* construct(Args&&... args)
  noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    T* ptr = allocate<T>(1u);
    if (!ptr) {
      return ptr;
    }
    std::construct_at(ptr, std::forward<Args>(args)...);
    return ptr;
  }

  template<typename T>
  constexpr void destroy(T* ptr)
  noexcept(std::is_nothrow_destructible_v<T>)
  {
    ptr->~T();
    deallocate(ptr, 1u);
  }

public:
  constexpr size_t size() const noexcept { return _used; }
  constexpr size_t capacity() const noexcept { return BUFFER_SIZE; }
  constexpr void* data() noexcept { return static_cast<void*>(&_buffer[0]); }
    
private:
  uint8 _buffer[BUFFER_SIZE];
  size_t _used;
};

} // namespace ntf
