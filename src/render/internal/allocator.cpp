#include "allocator.hpp"

namespace ntf::render {

static malloc_funcs base_alloc {
  .user_ptr = nullptr,
  .mem_alloc = malloc_pool::malloc_fn,
  .mem_free = malloc_pool::free_fn,
};

ctx_alloc::ctx_alloc(const malloc_funcs& funcs, linked_arena&& arena) noexcept:
  _arena{std::move(arena)},
  _user_ptr{funcs.user_ptr},
  _malloc{funcs.mem_alloc}, _free{funcs.mem_free} {}

auto ctx_alloc::make_alloc(weak_cptr<malloc_funcs> alloc_in,
                           size_t arena_size) -> uptr_t<ctx_alloc>
{
  ctx_alloc* ptr;
  auto& alloc = alloc_in ? *alloc_in : base_alloc;
  try {
    ptr = static_cast<ctx_alloc*>(std::invoke(alloc.mem_alloc, alloc.user_ptr,
                                              sizeof(ctx_alloc), alignof(ctx_alloc)));
    if (ptr) {
      auto arena = linked_arena::from_extern({
        .user_ptr = alloc.user_ptr,
        .mem_alloc = alloc.mem_alloc,
        .mem_free = alloc.mem_free
      }, arena_size);
      if (!arena){
        std::destroy_at(ptr);
        std::invoke(alloc.mem_free, alloc.user_ptr, ptr, sizeof(ctx_alloc));
        ptr = nullptr;
      } else {
        std::construct_at(ptr, alloc, std::move(*arena));
      }
    }
  } catch (...) {
    ptr = nullptr;
  }

  return uptr_t<ctx_alloc>{ptr, alloc_del_t<ctx_alloc>{alloc.user_ptr, alloc.mem_free}};
}

void* ctx_alloc::allocate(size_t size, size_t alignment) {
  void* ptr = std::invoke(_malloc, _user_ptr, size, alignment);
  if (!ptr) {
    throw std::bad_alloc{};
  }
  return ptr;
}

void ctx_alloc::deallocate(void* mem, size_t sz) {
  std::invoke(_free, _user_ptr, mem, sz);
}

void ctx_alloc::arena_clear() {
  _arena.clear();
}

void* ctx_alloc::arena_allocate(size_t size, size_t alignment) {
  void* ptr = _arena.allocate(size, alignment);
  if (!ptr) {
    throw std::bad_alloc{};
  }
  return ptr;
}

} // namespace ntf::Render
