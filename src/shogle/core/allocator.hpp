#pragma once

#include <shogle/core/common.hpp>

#include <memory>
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <list>

namespace ntf {

template<typename T, typename Allocator>
struct rebind_alloc {
  using type = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
};

template<typename T, typename Allocator>
using rebind_alloc_t = typename rebind_alloc<T, Allocator>::type;

// For using allocators in STL containers
// Store an allocator somewhere, create a stateless
// class that wraps it, and then pass it to the adapter
template<typename T, typename Allocator>
class allocator_adapter {
public:
  using allocator_type = Allocator;

  using value_type = T;
  using pointer = T*;

 public:
  [[nodiscard]] pointer allocate(std::size_t n);
  void deallocate(pointer ptr, [[maybe_unused]] std::size_t n);

  template<typename... Args>
  void construct(pointer ptr, Args&&... args);

  void destroy(pointer ptr);

public:
  bool operator==(const allocator_adapter& rhs) const noexcept;
  bool operator!=(const allocator_adapter& rhs) const noexcept;
};

template<std::size_t min_page_size>
class memory_arena {
private:
  struct page_header {
    std::size_t size;
    void* data;
  };

public:
  memory_arena() = default;
  memory_arena(std::size_t start_size);

public:
  void init(std::size_t start_size = min_page_size);

  [[nodiscard]] void* allocate(std::size_t size, std::size_t align);
  void deallocate([[maybe_unused]] void* ptr) {}

  void reset();

public:
  std::size_t used() const noexcept { return _used; }
  std::size_t allocated() const noexcept { return _allocated; }
  std::size_t page_count() const noexcept { return _pages.size(); }

private:
  page_header* _insert_page(std::size_t size);
  void _clear_pages();

private:
  std::list<page_header> _pages;
  std::size_t _used{0};
  std::size_t _allocated{0};
  std::size_t _page_offset{0};

public:
  ~memory_arena() noexcept;
  NTF_DISABLE_COPY(memory_arena);
};

class memory_stack {
public:
  memory_stack() = default;
  memory_stack(std::size_t size);

public:
  void init(std::size_t size);

  [[nodiscard]] void* allocate(std::size_t size, std::size_t align);
  void deallocate([[maybe_unused]] void* ptr) {}

  void resize(std::size_t new_size);
  void clear();
  void reset();

public:
  std::size_t used() const noexcept { return _offset; }
  std::size_t allocated() const noexcept { return _allocated; }

private:
  void* _create_page(std::size_t size);
  void _clear_page();

private:
  std::size_t _allocated{0};
  std::size_t _offset{0};
  void* _page{nullptr};

public:
  NTF_DECLARE_MOVE_ONLY(memory_stack);
};

} // namespace ntf

#ifndef NTF_ALLOCATOR_INL
#include <shogle/core/allocator.inl>
#endif
