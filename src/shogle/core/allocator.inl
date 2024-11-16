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

template<std::size_t min_page_size>
memory_arena<min_page_size>::memory_arena(std::size_t start_size) { init(start_size); }

template<std::size_t min_page_size>
memory_arena<min_page_size>::~memory_arena() noexcept { _clear_pages(); }

template<std::size_t min_page_size>
void memory_arena<min_page_size>::init(std::size_t start_size) { 
  assert(page_count() == 0);
  _insert_page(start_size);
}

template<std::size_t min_page_size>
void* memory_arena<min_page_size>::allocate(std::size_t size, std::size_t align) {
  assert(page_count() > 1 && size > 0);
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

template<std::size_t min_page_size>
void memory_arena<min_page_size>::reset() {
  assert(page_count() > 1);
  std::size_t used = _used;
  _used = 0;
  _allocated = 0;

  _clear_pages();
  _insert_page(used);
}

template<std::size_t min_page_size>
auto memory_arena<min_page_size>::_insert_page(std::size_t sz) -> page_header* {
  sz = std::max(sz, min_page_size); // Allocate at least min_page_size bytes

  _pages.emplace_back(page_header {
    .size = sz,
    .data = std::malloc(sz),
  });
  _page_offset = 0;
  _allocated += sz;

  return &_pages.back();
}

template<std::size_t min_page_size>
void memory_arena<min_page_size>::_clear_pages() {
  while (page_count() > 0) {
    auto& page = _pages.back();
    std::free(page.data);
    _pages.pop_back();
  }
}

inline memory_stack::memory_stack(std::size_t size) { init(size); }

inline memory_stack::~memory_stack() noexcept { _clear_page(); }

inline void memory_stack::init(std::size_t size) { _create_page(size); }

inline void* memory_stack::allocate(std::size_t size, std::size_t align) {
  assert(_allocated > 0 && _page && size > 0);
  std::size_t available = _allocated - _offset;
  std::size_t adjustment = impl::align_fw_adjust(impl::ptr_add(_page, _offset), align);
  std::size_t required = size + adjustment;

  assert(available > required);
  void* mem = impl::ptr_add(_page, _offset+adjustment);
  _offset += required;

  return mem;
}

inline void memory_stack::resize(std::size_t new_size) {
  _clear_page();
  _page = nullptr;
  _offset = 0;
  _allocated = 0;
  _create_page(new_size);
}

inline void memory_stack::clear() { _offset = 0; }

inline void memory_stack::reset() { resize(_allocated); }

inline void* memory_stack::_create_page(std::size_t size) {
  assert(_allocated == 0 && !_page && size > 0);
  _page = std::malloc(size);
  _allocated = size;
  return _page;
}

inline void memory_stack::_clear_page() {
  std::free(_page); // should work with nullptr?
}

} // namespace ntf
