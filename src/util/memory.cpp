#include <shogle/util/memory.hpp>

#include <sys/mman.h>
#include <unistd.h>

#ifdef SHOGLE_DISABLE_INTERNAL_LOGS
#define ALLOC_LOG(...)
#else
#define ALLOC_LOG(fmt_, ...) \
  ::ntf::logger::verbose("[ShOGLE][util/memory.cpp:{}] " fmt_, NTF_LINE __VA_OPT__(, ) __VA_ARGS__)
#endif

namespace shogle {

namespace {

const size_t PAGE_SIZE = static_cast<size_t>(sysconf(_SC_PAGE_SIZE));

size_t next_page_mult(size_t sz) noexcept {
  return PAGE_SIZE * std::ceil(static_cast<float>(sz) / static_cast<float>(PAGE_SIZE));
}

} // namespace

default_mem_alloc& default_mem_alloc::instance() noexcept {
  static default_mem_alloc alloc;
  return alloc;
}

size_t default_mem_alloc::page_size() noexcept {
  return PAGE_SIZE;
}

size_t default_mem_alloc::next_page_multiple(size_t size) noexcept {
  return next_page_mult(size);
}

void* default_mem_alloc::allocate(size_t size, size_t alignment) {
  return std::aligned_alloc(alignment, size);
}

void default_mem_alloc::deallocate(void* ptr, size_t size) noexcept {
  NTF_UNUSED(size);
  std::free(ptr);
}

std::pair<void*, size_t> default_mem_alloc::bulk_allocate(size_t size, size_t alignment) {
  NTF_UNUSED(alignment); // assume mmap is always 8 byte aligned
  const size_t mapping_size = std::min(PAGE_SIZE, next_page_mult(size));
  void* ptr = mmap(nullptr, mapping_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
  return {ptr, mapping_size};
}

void default_mem_alloc::bulk_deallocate(void* ptr, size_t size) noexcept {
  int ret = munmap(ptr, size);
  NTF_UNUSED(ret);
}

namespace {

constexpr size_t align_fw_adjust(void* ptr, size_t align) noexcept {
  uintptr_t iptr = std::bit_cast<uintptr_t>(ptr);
  // return ((iptr - 1u + align) & -align) - iptr;
  return align - (iptr & (align - 1u));
}

constexpr void* ptr_add(void* p, uintptr_t sz) noexcept {
  return std::bit_cast<void*>(std::bit_cast<uintptr_t>(p) + sz);
}

struct arena_header {
  arena_header* next;
  arena_header* prev;
  size_t size;
  size_t used;
};

std::pair<void*, size_t> init_arena(mem_alloc_interface& alloc, size_t initial_size) {
  const size_t block_size =
    std::max(next_page_mult(initial_size + sizeof(arena_header)), PAGE_SIZE);
  auto data = alloc.bulk_allocate(block_size, alignof(arena_header));
  SHOGLE_THROW_IF(!data.first, std::bad_alloc());
  ALLOC_LOG("Scratch arena initialized [alloc: {}, ptr: {}, size: {} bytes]", fmt::ptr(&alloc),
            fmt::ptr(data.first), data.second);
  arena_header* block = static_cast<arena_header*>(data.first);
  block->next = nullptr;
  block->prev = nullptr;
  block->size = data.second;
  block->used = 0u;
  return data;
}

void free_arena(mem_alloc_interface& alloc, void* block_) noexcept {
  arena_header* block = static_cast<arena_header*>(block_);
  while (block->next) {
    block = block->next;
  };
  while (block) {
    arena_header* prev = block->prev;
    const size_t size = block->size;
    alloc.bulk_deallocate(static_cast<void*>(block), size);
    block = prev;
  }
}

} // namespace

scratch_arena::scratch_arena(create_t, mem_alloc_interface& alloc, void* block,
                             size_t block_size) noexcept :
    _alloc(&alloc), _block(block), _total_used(), _allocated(block_size) {}

scratch_arena::scratch_arena(mem_alloc_interface& alloc, size_t initial_size) :
    _alloc(&alloc), _total_used() {
  std::tie(_block, _allocated) = init_arena(alloc, initial_size);
}

ntf::expected<scratch_arena, std::bad_alloc>
scratch_arena::from_size(mem_alloc_interface& alloc, size_t initial_size) noexcept {
  try {
    auto [block, block_size] = init_arena(alloc, initial_size);
    return {ntf::in_place, create_t{}, alloc, block, block_size};
  } catch (...) {
    return {ntf::unexpect};
  }
}

ntf::expected<scratch_arena, std::bad_alloc>
scratch_arena::from_size(size_t initial_size) noexcept {
  return ::shogle::scratch_arena::from_size(default_mem_alloc::instance(), initial_size);
}

void* scratch_arena::allocate(size_t size, size_t alignment) {
  arena_header* block = static_cast<arena_header*>(_block);
  const auto try_acquire_arena_block = [&]() -> arena_header* {
    arena_header* next_block = block->next;
    while (next_block) {
      void* data_init = ptr_add(next_block, sizeof(arena_header));
      const size_t padding = align_fw_adjust(data_init, alignment);
      if (next_block->size >= size + padding) {
        return next_block;
      }
      next_block = next_block->next;
    }

    const size_t block_size = std::max(next_page_mult(size + sizeof(arena_header)), PAGE_SIZE);
    void* mem = _alloc->allocate(block_size, alignof(arena_header));
    SHOGLE_THROW_IF(!mem, std::bad_alloc());
    ALLOC_LOG("Scratch arena resized [alloc: {}, ptr: {} -> {}, size: {} bytes -> {} bytes]",
              fmt::ptr(_alloc), fmt::ptr(_block), fmt::ptr(mem), _allocated,
              _allocated + block_size);

    arena_header* new_block = static_cast<arena_header*>(mem);
    block->next = nullptr;
    block->prev = block;
    block->size = block_size;
    _allocated += block_size;
    block->prev->next = new_block;
    return new_block;
  };

  void* data_init;
  size_t padding;
  size_t required;
  const auto calc_block = [&]() {
    data_init = ptr_add(block, sizeof(arena_header));
    padding = align_fw_adjust(ptr_add(data_init, block->used), alignment);
    required = size + padding;
  };

  const size_t available = block->size - block->used;
  calc_block();
  if (available < required) {
    block = try_acquire_arena_block();
    calc_block();
  }
  void* ptr = ptr_add(data_init, block->used + padding);
  _total_used += required;
  block->used += required;
  return ptr;
}

void scratch_arena::clear() noexcept {
  arena_header* block = static_cast<arena_header*>(_block);
  while (block->prev) {
    block->used = 0u;
    block = block->prev;
  }
  _block = static_cast<void*>(block);
  _total_used = 0u;
}

size_t scratch_arena::used() const noexcept {
  return _total_used;
}

size_t scratch_arena::allocated() const noexcept {
  return _allocated;
}

scratch_arena::~scratch_arena() noexcept {
  if (_block) {
    ALLOC_LOG("Scratch arena destroyed [alloc: {}, ptr: {}, size: {} bytes]", fmt::ptr(_alloc),
              fmt::ptr(_block), _allocated);
    free_arena(*_alloc, _block);
  }
}

scratch_arena::scratch_arena(scratch_arena&& other) noexcept :
    _alloc(other._alloc), _block(other._block), _total_used(other._total_used),
    _allocated(other._allocated) {
  other._block = nullptr;
}

scratch_arena& scratch_arena::operator=(scratch_arena&& other) noexcept {
  if (_block) {
    ALLOC_LOG("Scratch arena destroyed [alloc: {}, ptr: {}, size: {} bytes]", fmt::ptr(_alloc),
              fmt::ptr(_block), _allocated);
    free_arena(*_alloc, _block);
  }

  _alloc = other._alloc;
  _block = other._block;
  _total_used = other._total_used;
  _allocated = other._allocated;

  _block = nullptr;

  return *this;
}

} // namespace shogle
