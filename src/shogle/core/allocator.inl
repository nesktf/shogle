#define NTF_ALLOCATOR_INL
#include <shogle/core/allocator.hpp>
#undef NTF_ALLOCATOR_INL

namespace ntf {

template<typename T, typename Allocator>
auto allocator_adapter<T, Allocator>::allocate(std::size_t n) -> pointer {
  return static_cast<pointer>(allocator_type{}.allocate(n*sizeof(T)));
}

template<typename T, typename Allocator>
void allocator_adapter<T, Allocator>::deallocate(pointer ptr, std::size_t n) {
  allocator_type{}.deallocate(ptr, n);
}

template<typename T, typename Allocator>
template<typename... Args>
void allocator_adapter<T, Allocator>::construct(pointer ptr, Args&&... args) {
  new (ptr) T{std::forward<Args>(args)...};
}

template<typename T, typename Allocator>
void allocator_adapter<T, Allocator>::destroy(pointer ptr) {
  ptr->~T();
}

template<typename T, typename Allocator>
bool allocator_adapter<T, Allocator>::operator==(const allocator_adapter&) const noexcept {
  return true; // TODO: Actually compare both...
}

template<typename T, typename Allocator>
bool allocator_adapter<T, Allocator>::operator!=(const allocator_adapter& rhs) const noexcept {
  return !(*this == rhs);
}

template<std::size_t page_size>
void* memory_arena<page_size>::allocate(std::size_t required) {
  auto* page = &_pages.back();
  std::size_t available = page->size - _page_offset;

  if (available < required) {
    page = _insert_page(required);
  }

  void* mem = page->data + _page_offset;
  _used += required;
  _page_offset += required;

  return mem;
};

template<std::size_t page_size>
memory_arena<page_size>::~memory_arena() noexcept {
  while (_pages.size() > 0) {
    auto& page = _pages.back();
    std::free(page.data);
    _pages.pop_back();
  }
}

template<std::size_t page_size>
auto memory_arena<page_size>::_insert_page(std::size_t sz) -> page_header* {
  sz = std::max(sz, page_size); // Allocate at least page_size bytes

  _pages.emplace_back(page_header {
    .size = page_size,
    .data = static_cast<uint8_t*>(std::malloc(sz)),
  });
  _page_offset = 0;

  return &_pages.back();
}


} // namespace ntf
