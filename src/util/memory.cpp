#include <shogle/util/memory.hpp>

// TODO: Make this multiplatform
#ifdef __linux__
#include <sys/mman.h>
#include <unistd.h>
#else
#error "memory.cpp not defined for non linux os"
#endif

#include <cmath>

namespace shogle::mem {

#ifdef __linux__
size_t system_page_size() noexcept {
  static const size_t page_size = static_cast<size_t>(sysconf(_SC_PAGE_SIZE));
  return page_size;
}
#endif

namespace {

size_t next_page_size(size_t sz) noexcept {
  // Allocate one extra memory page just in case
  const size_t page_size = system_page_size();
  return page_size * static_cast<size_t>(
                       (1.f + std::ceil(static_cast<f32>(sz) / static_cast<f32>(page_size))));
}

struct default_bulk_alloc {
public:
#ifdef __linux__
  void* bulk_allocate(size_t& size, size_t align) {
    SHOGLE_UNUSED(align);
    const size_t mapping_sz = next_page_size(size);
    void* ptr = mmap(nullptr, mapping_sz, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
    SHOGLE_THROW_IF(!ptr, std::bad_alloc());
    size = mapping_sz;
    return ptr;
  }

  void bulk_deallocate(void* ptr, size_t size) noexcept {
    int ret = munmap(ptr, size);
    SHOGLE_UNUSED(ret);
  }
#endif
  bool is_equal(const default_bulk_alloc&) const noexcept { return true; }
} g_bulk_alloc;

static_assert(::shogle::mem::bulk_memory_resource_type<default_bulk_alloc>);

struct arena_header {
  bulk_memory_resource res;
  arena_header* next;
  arena_header* prev;
  size_t size;
  size_t used;
};

std::pair<void*, size_t> init_arena(const bulk_memory_resource* res,
                                    size_t initial_size) noexcept {
  size_t block_size = next_page_size(initial_size + sizeof(arena_header));
  void* ptr;
  try {
    if (res) {
      ptr = res->allocate(block_size, alignof(arena_header));
    } else {
      ptr = g_bulk_alloc.bulk_allocate(block_size, alignof(arena_header));
    }
  } catch (...) {
    return {nullptr, 0};
  }
  if (!ptr) {
    return {nullptr, 0};
  }
  arena_header* header = static_cast<arena_header*>(ptr);
  header->res = res ? *res : bulk_memory_resource(g_bulk_alloc);
  header->next = nullptr;
  header->prev = nullptr;
  header->size = block_size;
  header->used = 0u;
  return {ptr, block_size};
}

void free_arena(void* arena_block) noexcept {
  arena_header* header = static_cast<arena_header*>(arena_block);
  while (header->next) {
    header = header->next;
  }
  while (header) {
    arena_header* prev = header->prev;
    const size_t block_size = header->size;
    bulk_memory_resource res = header->res;
    res.deallocate(static_cast<void*>(header), block_size);
    header = prev;
  }
}

} // namespace

scratch_arena::scratch_arena(create_t, void* data, size_type block_size) noexcept :
    _data(data), _used(), _allocated(block_size) {}

scratch_arena::scratch_arena(size_type initial_size) : _used() {
  std::tie(_data, _allocated) = init_arena(nullptr, initial_size);
  SHOGLE_THROW_IF(!_data, std::bad_alloc());
}

scratch_arena::scratch_arena(size_type initial_size, const bulk_memory_resource& res) : _used() {
  std::tie(_data, _allocated) = init_arena(&res, initial_size);
  SHOGLE_THROW_IF(!_data, std::bad_alloc());
}

optional<scratch_arena> scratch_arena::with_initial_size(size_type initial_size) noexcept {
  auto [ptr, sz] = init_arena(nullptr, initial_size);
  if (!ptr) {
    return nullopt;
  }
  return {in_place, create_t{}, ptr, sz};
}

optional<scratch_arena>
scratch_arena::with_memory_resource(size_type initial_size,
                                    const bulk_memory_resource& res) noexcept {
  auto [ptr, sz] = init_arena(&res, initial_size);
  if (!ptr) {
    return nullopt;
  }
  return {in_place, create_t{}, ptr, sz};
}

void* scratch_arena::allocate(size_type size, size_type alignment) {
  SHOGLE_ASSERT(_data != nullptr, "scratch_arena use after move");
  arena_header* header = static_cast<arena_header*>(_data);
  const auto acquire_new_block = [&]() {
    // First we try to fit our data in an already existing memory block
    arena_header* next = header->next;
    while (next) {
      void* data_init = ptr_add(static_cast<void*>(next), sizeof(arena_header));
      const size_t pad = align_fw_adjust(data_init, alignment);
      if (next->size >= size + pad) {
        header = next;
        _data = static_cast<void*>(next);
        return;
      }
      next = next->next;
    }

    // If we didn't find an appropiate block, we allocate a new one
    size_t block_size = next_page_size(size + sizeof(arena_header));
    void* mem = header->res.allocate(block_size, alignof(arena_header));
    SHOGLE_THROW_IF(!mem, std::bad_alloc());

    // Initialize the new block header
    arena_header* new_header = static_cast<arena_header*>(mem);
    new_header->res = header->res;
    new_header->next = nullptr;
    new_header->prev = header;
    new_header->prev->next = new_header;
    _allocated += block_size;
    header = new_header;
    _data = mem;
  };
  void* data_init;
  size_t pad, required;
  const auto calc_block = [&]() {
    data_init = ptr_add(_data, sizeof(arena_header));
    pad = align_fw_adjust(ptr_add(data_init, header->used), alignment);
    required = size + pad;
  };

  const size_t available = header->size - header->used;
  calc_block();
  if (available < required) {
    acquire_new_block();
    calc_block();
  }
  void* ptr = ptr_add(data_init, header->used + pad);
  _used += required;
  header->used += required;
  return ptr;
}

void scratch_arena::clear() noexcept {
  arena_header* header = static_cast<arena_header*>(_data);
  while (header->prev) {
    header->used = 0u;
    header = header->prev;
  }
  _data = static_cast<void*>(header);
  _used = 0u;
}

// TODO: this thing
bool scratch_arena::is_equal(const scratch_arena& other) const noexcept {
  return false;
}

scratch_arena::~scratch_arena() noexcept {
  if (_data) {
    free_arena(_data);
  }
}

scratch_arena::scratch_arena(scratch_arena&& other) noexcept :
    _data(other._data), _used(other._used), _allocated(other._allocated) {
  other._data = nullptr;
}

scratch_arena& scratch_arena::operator=(scratch_arena&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  if (_data) {
    free_arena(_data);
  }

  _data = other._data;
  _used = other._used;
  _allocated = other._allocated;

  other._data = nullptr;

  return *this;
}

} // namespace shogle::mem
