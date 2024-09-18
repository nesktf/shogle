#define NTF_ALLOCATOR_INL
#include <shogle/core/allocator.hpp>
#undef NTF_ALLOCATOR_INL

namespace ntf {

namespace impl {

inline std::size_t align_fw_adjust(void* ptr, std::size_t align) noexcept {
  std::uintptr_t iptr = reinterpret_cast<uintptr_t>(ptr);
  // return ((iptr - 1u + align) & -align) - iptr;
  return align - (iptr & (align - 1u));
}

inline void* ptr_add(void* p, std::uintptr_t sz) noexcept {
  return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + sz);
}

} // namespace impl

template<typename T, typename Allocator>
auto allocator_adapter<T, Allocator>::allocate(std::size_t n) -> pointer {
  return reinterpret_cast<pointer>(allocator_type{}.allocate(n*sizeof(T), alignof(T)));
}

template<typename T, typename Allocator>
void allocator_adapter<T, Allocator>::deallocate(pointer ptr, [[maybe_unused]] std::size_t n) {
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
memory_arena<page_size>::memory_arena(std::size_t start_size) { _insert_page(start_size); }

template<std::size_t page_size>
memory_arena<page_size>::~memory_arena() noexcept { _clear_pages(); }

template<std::size_t page_size>
void* memory_arena<page_size>::allocate(std::size_t size, std::size_t align) {
  auto* page = &_pages.back();

  std::size_t available = page->size - _page_offset;
  std::size_t adjustment = impl::align_fw_adjust(impl::ptr_add(page->data, _page_offset), align);
  std::size_t required = size + adjustment;

  if (available < required) {
    page = _insert_page(required); // Will waste the last few free bytes in the block
    adjustment = impl::align_fw_adjust(page->data, align);
    required = size + adjustment;
  }

  void* mem = impl::ptr_add(page->data, _page_offset+adjustment);
  _used += required;
  _page_offset += required;

  return mem;
};

template<std::size_t page_size>
void memory_arena<page_size>::reset() {
  std::size_t used = _used;
  _used = 0;
  _page_offset = 0;

  if (page_count() > 1) {
    _clear_pages();
    _allocated = 0;
    _insert_page(used);
  }
}

template<std::size_t page_size>
auto memory_arena<page_size>::_insert_page(std::size_t sz) -> page_header* {
  sz = std::max(sz, page_size); // Allocate at least page_size bytes

  _pages.emplace_back(page_header {
    .size = page_size,
    .data = std::malloc(sz),
  });
  _page_offset = 0;
  _allocated += sz;

  return &_pages.back();
}

template<std::size_t page_size>
void memory_arena<page_size>::_clear_pages() {
  while (page_count() > 0) {
    auto& page = _pages.back();
    std::free(page.data);
    _pages.pop_back();
  }
}

} // namespace ntf
