#define SHOGLE_STL_ARENA_INL
#include "./arena.hpp"
#undef SHOGLE_STL_ARENA_INL

namespace ntf {

template<typename P>
void basic_memory_arena<P>::init(std::size_t block) noexcept {
  NTF_ASSERT(!_block);
  _insert_block(block);
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
  return allocator_adaptor<T, P>{static_cast<T&>(*this)};
}

template<typename T>
void basic_memory_arena<T>::_reset() noexcept {
  _block = nullptr;
  _used = 0;
  _allocated = 0;
  _block_count = 0;
  _block_offset = 0; 
}

template<typename P>
auto basic_memory_arena<P>::_insert_block(std::size_t size) noexcept -> arena_block* {
  void* mem = std::malloc(sizeof(arena_block)+std::max(size, MIN_BLOCK_SIZE));
  NTF_ASSERT(mem, "Out of memory :(");

  arena_block* block = reinterpret_cast<arena_block*>(mem);
  std::construct_at(block, _block, size, impl::ptr_add(mem, sizeof(arena_block)));

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
  _reset();
}

template<typename P>
void* basic_memory_arena<P>::_block_mem(arena_block* block, std::size_t required,
                                        std::size_t adjustment) noexcept {
  NTF_ASSERT(block);

  void* mem = impl::ptr_add(block->data, _block_offset + adjustment);
  _used += required;
  _block_offset += required;

  return mem;
}

template<typename T>
basic_memory_arena<T>::~basic_memory_arena() noexcept { _clear_pages(); }

template<typename T>
basic_memory_arena<T>::basic_memory_arena(basic_memory_arena&& a) noexcept :
  _block(a._block),
  _block_count(a._block_count), _block_offset(a._block_offset),
  _used(a._used), _allocated(a._allocated) { a._reset(); }

template<typename T>
auto basic_memory_arena<T>::operator=(basic_memory_arena&& a) noexcept -> basic_memory_arena& {
  _clear_pages();

  _block = std::move(a._block);
  _block_count = std::move(a._block_count);
  _block_offset = std::move(a._block_offset);
  _used = std::move(a._used);
  _allocated = std::move(a._allocated);

  a._reset();

  return *this;
}

inline void* memory_arena::allocate(std::size_t size, std::size_t align) noexcept {
  NTF_ASSERT(_block, "Arena not initialized");
  NTF_ASSERT(size > 0, "Invalid allocation size");

  auto* block = _block;
  const std::size_t available = block->size - _block_offset;
  std::size_t adjustment = 
    impl::align_fw_adjust(impl::ptr_add(block->data, _block_offset), align);
  std::size_t required = size + adjustment;

  if (available < required) {
    block = _insert_block(required); // Wil waste the last few free bytes in the block
    adjustment = impl::align_fw_adjust(block->data, align);
    required = size + adjustment;
  }

  return _block_mem(block, required, adjustment);
}

inline void* memory_stack::allocate(std::size_t size, std::size_t align) noexcept {
  NTF_ASSERT(_block);
  NTF_ASSERT(size > 0, "Invalid allocation size");

  auto* block = _block;
  std::size_t available = block->size - _block_offset;
  std::size_t adjustment = impl::align_fw_adjust(impl::ptr_add(block->data, _block_offset), align);
  std::size_t required = size + adjustment;

  NTF_ASSERT(available > required 
             && (_used+required) <= _alloc_limit, "Memory stack limit reached");
  return _block_mem(block, required, adjustment);
}

inline void memory_stack::reset() noexcept {
  _used = 0;
  _block_offset = 0;
}

} // namespace ntf
