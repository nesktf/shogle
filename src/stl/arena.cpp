#include "./arena.hpp"

#include <sys/mman.h>
#include <unistd.h>

static const std::size_t page_size = static_cast<std::size_t>(sysconf(_SC_PAGE_SIZE));
static constexpr int mem_pflag = PROT_READ | PROT_WRITE;
static constexpr int mem_type = MAP_PRIVATE | MAP_ANONYMOUS;

// static std::size_t next_multiple(std::size_t sz) noexcept {
//   return page_size*std::ceil(static_cast<float>(sz)/static_cast<float>(page_size));
// }

static std::size_t align_fw_adjust(void* ptr, std::size_t align) noexcept {
  uintptr_t iptr = reinterpret_cast<uintptr_t>(ptr);
  return align - (iptr & (align - 1u));
  // return ((iptr - 1u + align) & -align) - iptr;
}

static void* ptr_add(void* p, uintptr_t sz) noexcept {
  return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + sz);
}

namespace ntf {

void mem_arena::init(std::uint64_t reserve) noexcept {
  _max_size = reserve;
  _base = mmap(nullptr, reserve, mem_pflag, mem_type, -1, 0);
}

void* mem_arena::allocate(std::size_t size, std::size_t align) noexcept {
  std::size_t padding = align_fw_adjust(ptr_add(_base, _allocated), align);
  void* ptr = ptr_add(_base, _allocated+padding);
  _allocated += padding+size;
  return ptr;
}

void mem_arena::reset() noexcept { _allocated = 0; }

void mem_arena::set_alloc(std::size_t size) noexcept { _allocated = size; }

void mem_arena::decrease_alloc(std::size_t size) noexcept { _allocated -= size; }

mem_arena::mem_arena(std::uint64_t reserve) noexcept { init(reserve); }

mem_arena::~mem_arena() noexcept {
  if (_base) {
    munmap(_base, _max_size);
  }
}

} // namespace ntf
