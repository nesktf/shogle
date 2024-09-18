#pragma once

#include <shogle/core/common.hpp>

#include <cstdlib>
#include <cstdint>
#include <list>

namespace ntf {

// For using allocators in STL containers
// Store an allocator somewhere, create a stateless
// class that wraps it, and then pass it to the adapter
template<typename T, typename Allocator>
class allocator_adapter {
public:
  using allocator_type = Allocator;

  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using const_reference = const T&;

  template<typename U>
  struct rebind {
    using other = allocator_adapter<U, allocator_type>;
  };

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


template<std::size_t page_size>
class memory_arena {
private:
  struct page_header {
    std::size_t size;
    void* data;
  };

public:
  memory_arena(std::size_t start_size = page_size) { _insert_page(start_size); };

public:
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
  NTF_DECLARE_MOVE_ONLY(memory_arena);
};

} // namespace ntf

#ifndef NTF_ALLOCATOR_INL
#include <shogle/core/allocator.inl>
#endif
