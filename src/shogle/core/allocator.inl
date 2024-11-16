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


template<typename P>
void basic_memory_arena<P>::init(std::size_t size) noexcept {
  if (_block_count == 0) {
    return;
  }
  _insert_block(size);
}

template<typename P>
auto basic_memory_arena<P>::_insert_block(std::size_t size) noexcept -> arena_block* {
  void* mem = std::malloc(sizeof(arena_block)+std::max(size, MIN_BLOCK_SIZE));
  NTF_ASSERT(mem, "Out of memory :(");

  arena_block* block = reinterpret_cast<arena_block*>(mem);
  std::construct_at(block, _block, size, ptr_add(mem, sizeof(arena_block)));

  _block = block;
  _block_offset = 0;
  _block_count++;

  _allocated += size;

  return block;
}

template<typename P>
void basic_memory_arena<P>::_clear_pages() noexcept {
  arena_block* curr = _block;
  while (curr) {
    curr = curr->next;
    curr->~arena_block(); // Needed if arena_block is trivial?
    std::free(curr);
  }
  _block = nullptr;
  _block_count = 0;
  _block_offset = 0;

  _allocated = 0;
  _used = 0;
}

template<typename P>
void basic_memory_arena<P>::clear(bool reallocate) noexcept {
  if (_block_count == 0) {
    return;
  }

  const std::size_t prev_allocated = _allocated;
  _clear_pages();
  if (reallocate) {
    _insert_block(prev_allocated);
  }
}

template<typename P>
template<typename T>
auto basic_memory_arena<P>::make_adaptor() -> allocator_adaptor<T, P> {
  return allocator_adaptor<T, P>{*this};
}

} // namespace impl

inline void* memory_arena::allocate(std::size_t size, std::size_t align) noexcept {
  NTF_ASSERT(_block_count >= 1, "Arena not initialized");
  NTF_ASSERT(size > 0, "Invalid allocation size");

  auto* block = _block;
  std::size_t available = block->size - _block_offset;
  std::size_t adjustment = impl::align_fw_adjust(impl::ptr_add(block->data, _block_offset), align);
  std::size_t required = size + adjustment;

  if (available < required) {
    block = _insert_block(required); // Wil waste the last few free bytes in the block
    adjustment = impl::align_fw_adjust(block->data, align);
    required = size + adjustment;
  }

  void* mem = impl::ptr_add(block->data, _block_offset + adjustment);
  _used += required;
  _block_offset += required;

  return mem;
}

inline void* memory_stack::allocate(std::size_t size, std::size_t align) noexcept {
  NTF_ASSERT(_block_count >= 1, "Memory stack not initialized");
  NTF_ASSERT(size > 0, "Invalid allocation size");

  auto* block = _block;
  std::size_t available = block->size - _block_offset;
  std::size_t adjustment = impl::align_fw_adjust(impl::ptr_add(block->data, _block_offset), align);
  std::size_t required = size + adjustment;

  NTF_ASSERT(available > required 
             && (_used+required) <= _alloc_limit, "Memory stack limit reached");
  void* mem = impl::ptr_add(block->data, _block_offset + adjustment);
  _used += required;
  _block_offset += required;

  return mem;
}

} // namespace ntf
