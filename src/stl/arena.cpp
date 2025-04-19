#include "./arena.hpp"

#include <sys/mman.h>
#include <unistd.h>

namespace ntf {

namespace {

static const size_t page_size = static_cast<std::size_t>(sysconf(_SC_PAGE_SIZE));
static constexpr int mem_pflag = PROT_READ | PROT_WRITE;
static constexpr int mem_type = MAP_PRIVATE | MAP_ANONYMOUS;

size_t next_page_mult(size_t sz) noexcept {
  return page_size*std::ceil(static_cast<float>(sz)/static_cast<float>(page_size));
}

struct arena_header {
  arena_header* next;
  arena_header* prev;
  size_t size;
};
static constexpr size_t MIN_BLOCK_SIZE = kibs(4);

std::pair<arena_header*, size_t> try_acquire_block(size_t size, void* prev = nullptr) {
  const size_t block_sz = std::max(next_page_mult(size+sizeof(arena_header)), MIN_BLOCK_SIZE);
  void* mem = std::malloc(block_sz);
  if (!mem) {
    return std::make_pair(nullptr, 0u);
  }
  arena_header* block = static_cast<arena_header*>(mem);
  block->next = nullptr;
  block->prev = static_cast<arena_header*>(prev);
  block->size = block_sz;
  return std::make_pair(block, block_sz);
}

void free_blocks(void* block_) {
  if (!block_) {
    return;
  }
  arena_header* block = static_cast<arena_header*>(block_);
  while (block->next) {
    block = block->next;
  }
  while (block) {
    arena_header* prev = block->prev;
    std::free(block);
    block = prev;
  }
}

} // namespace


void* arena_block_manager::allocate(size_t size, size_t alignment) noexcept {
  const size_t available = _allocated-_used;
  const size_t padding = align_fw_adjust(ptr_add(_block, _used), alignment);
  const size_t required = padding+size;

  if (available < required) {
    return nullptr;
  }

  void* ptr = ptr_add(_block, _used+padding);
  _used += required;
  return ptr;
}

void arena_block_manager::clear() noexcept {
  _used = 0u;
}

expected<linked_arena, error<void>> linked_arena::from_size(size_t size) noexcept {
  auto [block, block_sz] = try_acquire_block(size);
  if (!block) {
    return unexpected{error<void>{"Allocation failure"}};
  }
  return linked_arena{block, block_sz};
}

void* linked_arena::allocate(size_t size, size_t alignment) noexcept {
  arena_header* block = static_cast<arena_header*>(_block);
  void* data_init = ptr_add(block, sizeof(arena_header));

  const size_t available = block->size-_block_used;
  size_t padding = align_fw_adjust(ptr_add(data_init, _block_used), alignment);
  size_t required = size+padding;

  if (available < required) {
    if (block->next) {
      block = block->next;
    } else if (auto [ptr, block_sz] = try_acquire_block(required, _block); ptr != nullptr) {
      block = ptr;
      _allocated += block_sz;
    } else {
      return nullptr;
    }

    data_init = ptr_add(block, sizeof(arena_header));
    padding = align_fw_adjust(data_init, alignment);
    required = size+padding;
    _block_used = 0u;
    _block = block; // Will waste the last few free bytes in the block
  }

  void* ptr = ptr_add(data_init, _block_used+padding);
  _total_used += required;
  _block_used += required;
  return ptr;
}

void linked_arena::clear() noexcept {
  arena_header* block = static_cast<arena_header*>(_block);
  while (block->prev != nullptr) {
    block = block->prev;
  }
  _block = block;
  _total_used = 0u;
  _block_used = 0u;
}

linked_arena::~linked_arena() noexcept { free_blocks(_block); }

linked_arena::linked_arena(linked_arena&& other) noexcept :
  _block{std::move(other._block)},
  _block_used{std::move(other._block_used)},
  _total_used{std::move(other._total_used)},
  _allocated{std::move(other._allocated)} { other._block = nullptr; }

linked_arena& linked_arena::operator=(linked_arena&& other) noexcept {
  free_blocks(_block);

  _block = std::move(other._block);
  _block_used = std::move(other._block_used);
  _total_used = std::move(other._total_used);
  _allocated = std::move(other._allocated);

  other._block = nullptr;

  return *this;
}

expected<fixed_arena, error<void>> fixed_arena::from_size(size_t size) noexcept {
  void* ptr = mmap(nullptr, size, mem_pflag, mem_type, -1, 0);
  if (!ptr) {
    return unexpected{error<void>{"Allocation failure"}};
  }
  return fixed_arena{ptr, size};
}

fixed_arena::~fixed_arena() noexcept {
  if (data()) {
    munmap(data(), capacity());
  }
}

fixed_arena::fixed_arena(fixed_arena&& other) noexcept :
  arena_block_manager{static_cast<arena_block_manager&&>(other)}
{
  other._block = nullptr;
}

fixed_arena& fixed_arena::operator=(fixed_arena&& other) noexcept {
  if (data()) {
    munmap(data(), capacity());
  }

  arena_block_manager::operator=(static_cast<arena_block_manager&&>(other));

  other._block = nullptr;

  return *this;
}

} // namespace ntf
