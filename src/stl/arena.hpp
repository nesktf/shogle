#pragma once

#include "./allocator.hpp"

namespace ntf {

template<typename P>
class basic_memory_arena {
public:
  template<typename T>
  using bind_adaptor = allocator_adaptor<T, P>;

protected:
  struct arena_block {
    arena_block* next;
    std::size_t size;
    void* data;
  };

  static constexpr std::size_t MIN_BLOCK_SIZE = 4096;

public:
  basic_memory_arena() = default;
  basic_memory_arena(std::size_t block) noexcept { init(block); }

public:
  void init(std::size_t block) noexcept;
  void deallocate(void*, std::size_t) noexcept {}
  void clear(bool reallocate = false) noexcept;

public:
  std::size_t used() const noexcept { return _used; }
  std::size_t allocated() const noexcept { return _allocated; }
  std::size_t blocks() const noexcept { return _block_count; }

  template<typename T>
  allocator_adaptor<T, P> make_adaptor();

protected:
  arena_block* _insert_block(std::size_t size) noexcept;
  void _clear_pages() noexcept;
  void _reset() noexcept;
  void* _block_mem(arena_block* block, std::size_t required, std::size_t adjustment) noexcept;

protected:
  arena_block* _block{nullptr};
  std::size_t _block_count{0}, _block_offset{0};

  std::size_t _used{0}, _allocated{0};

public:
  NTF_DECLARE_MOVE_ONLY(basic_memory_arena);
};


class memory_arena final : public basic_memory_arena<memory_arena> {
public:
  using basic_memory_arena<memory_arena>::basic_memory_arena;

public:
  [[nodiscard]] void* allocate(std::size_t size, std::size_t align) noexcept;
};


class memory_stack final : public basic_memory_arena<memory_stack> {
private:
  static constexpr std::size_t DEFAULT_ALLOC_LIMIT = 4096;

public:
  memory_stack() = default;
  memory_stack(std::size_t alloc_limit) noexcept :
    basic_memory_arena<memory_stack>(), _alloc_limit(alloc_limit) {}
  memory_stack(std::size_t alloc_limit, std::size_t block) noexcept :
    basic_memory_arena<memory_stack>(block), _alloc_limit(alloc_limit) {}

public:
  [[nodiscard]] void* allocate(std::size_t size, std::size_t align) noexcept;
  void reset() noexcept;

public:
  std::size_t limit() const { return _alloc_limit; }
  std::size_t remaining() const { return limit() - used(); }

private:
  std::size_t _alloc_limit{DEFAULT_ALLOC_LIMIT};
};

} // namespace ntf

#ifndef SHOGLE_STL_ARENA_INL
#include "./arena.inl"
#endif
