#pragma once

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
  pointer allocate(std::size_t n);
  void deallocate(pointer ptr, std::size_t n);

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
    std::uint8_t* data;
  };

public:
  memory_arena(std::size_t start_size = page_size) { _insert_page(start_size); }

public:
  void* allocate(std::size_t required);
  void deallocate(void*, std::size_t) {}

private:
  page_header* _insert_page(std::size_t size);

private:
  std::list<page_header> _pages;
  std::size_t _used{0};
  std::size_t _page_offset{0};

public:
  ~memory_arena() noexcept;
  memory_arena(memory_arena&&) = default;
  memory_arena(const memory_arena&) = delete;
  memory_arena& operator=(memory_arena&&) = default;
  memory_arena& operator=(const memory_arena&) = delete;
};

} // namespace ntf

#ifndef NTF_ALLOCATOR_INL
#include <shogle/core/allocator.inl>
#endif
