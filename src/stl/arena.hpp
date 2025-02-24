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
    size_t size;
    void* data;
  };

  static constexpr size_t MIN_BLOCK_SIZE = 4096;

public:
  basic_memory_arena() = default;
  basic_memory_arena(size_t block) noexcept { init(block); }

public:
  void init(size_t block) noexcept;
  void deallocate(void*, size_t) noexcept {}
  void clear(bool reallocate = false) noexcept;

public:
  size_t used() const noexcept { return _used; }
  size_t allocated() const noexcept { return _allocated; }
  size_t blocks() const noexcept { return _block_count; }

  template<typename T>
  allocator_adaptor<T, P> make_adaptor();

protected:
  arena_block* _insert_block(size_t size) noexcept;
  void _clear_pages() noexcept;
  void _reset() noexcept;
  void* _block_mem(arena_block* block, size_t required, size_t adjustment) noexcept;

protected:
  arena_block* _block{nullptr};
  size_t _block_count{0}, _block_offset{0};

  size_t _used{0}, _allocated{0};

public:
  NTF_DECLARE_MOVE_ONLY(basic_memory_arena);
};


class memory_arena final : public basic_memory_arena<memory_arena> {
public:
  using basic_memory_arena<memory_arena>::basic_memory_arena;

public:
  [[nodiscard]] void* allocate(size_t size, size_t align) noexcept;
};


class memory_stack final : public basic_memory_arena<memory_stack> {
private:
  static constexpr size_t DEFAULT_ALLOC_LIMIT = 4096;

public:
  memory_stack() = default;
  memory_stack(size_t alloc_limit) noexcept :
    basic_memory_arena<memory_stack>(), _alloc_limit(alloc_limit) {}
  memory_stack(size_t alloc_limit, size_t block) noexcept :
    basic_memory_arena<memory_stack>(block), _alloc_limit(alloc_limit) {}

public:
  [[nodiscard]] void* allocate(size_t size, size_t align) noexcept;
  void reset() noexcept;

public:
  size_t limit() const { return _alloc_limit; }
  size_t remaining() const { return limit() - used(); }

private:
  size_t _alloc_limit{DEFAULT_ALLOC_LIMIT};
};


constexpr inline uint64_t kibs(uint64_t count) { return count << 10; }
constexpr inline uint64_t mibs(uint64_t count) { return count << 20; }
constexpr inline uint64_t gibs(uint64_t count) { return count << 30; }
constexpr inline uint64_t tibs(uint64_t count) { return count << 40; }

class mem_arena {
public:
  mem_arena() = default;
  explicit mem_arena(std::uint64_t reserve) noexcept;

public:
  void init(std::uint64_t reserve) noexcept;

public:
  template<typename T>
  T* allocate(std::uint32_t count) noexcept {
    return reinterpret_cast<T*>(allocate(count*sizeof(T), alignof(T)));
  }

  void* allocate(size_t size, size_t align) noexcept;
  void deallocate(void*) noexcept {}

  void reset() noexcept;
  void set_alloc(size_t size) noexcept;
  void decrease_alloc(size_t size) noexcept;

public:
  size_t allocated() const { return _allocated; }

private:
  void* _base{nullptr};
  std::uint64_t _max_size{0};
  size_t _allocated{0};

public:
  NTF_DECLARE_MOVE_ONLY(mem_arena);
};

template<typename T>
class mem_pool {
private:
  struct node_type {
    node_type() : dummy(), next(nullptr), prev(nullptr) {}

    template<typename... Args>
    void construct(Args&&... args) {
      new (&obj) T{std::forward<Args>(args)...};
    }

    void destroy() {
      obj.~T();
    }

    union {
      T obj;
      uint8_t dummy;
    };
    node_type *next, *prev;
  };

  class handle {
  private:
    handle(mem_pool& pool, size_t pos) :
      _pool(&pool), _pos(pos) {}
      
  public:
    T& operator*() {

    }

  private:
    mem_pool* _pool;
    size_t _pos;

  private:
    friend class mem_pool;
  };

public:
  mem_pool() = default;
  explicit mem_pool(std::uint64_t reserve) noexcept { init(reserve); }

public:
  void init(std::uint64_t reserve) noexcept {
    _arena.init(reserve);
  }

  template<typename... Args>
  T* construct(Args&&... args) noexcept {
    if (_free_list) {
      node_type* node = _free_list;

      if (node->next) {
        node->next->prev = nullptr;
      }
      _free_list = node->next;
      node->next = nullptr;
      node->prev = nullptr;

      node->construct(std::forward<Args>(args)...);
      return &node->obj;
    }

    node_type* node = _arena.allocate<node_type>(1);
    new (node) node_type{};
    node->construct(std::forward<Args>(args)...);
    return &node->obj;
  }

  void destroy(T* obj) noexcept {
    node_type* node = reinterpret_cast<node_type*>(obj);
    node->destroy();

    node->prev = nullptr;
    node->next = _free_list;
    if (node->next) {
      node->next->prev = node;
    }
    _free_list = node;
  }

public:
  size_t allocated() const { return _arena.allocated(); }

private:
  mem_arena _arena{};
  node_type* _free_list{nullptr};
};

template<typename P>
void basic_memory_arena<P>::init(size_t block) noexcept {
  NTF_ASSERT(!_block);
  _insert_block(block);
}

template<typename P>
void basic_memory_arena<P>::clear(bool reallocate) noexcept {
  if (_block_count == 0) {
    return;
  }

  const size_t prev_allocated = _allocated;
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
auto basic_memory_arena<P>::_insert_block(size_t size) noexcept -> arena_block* {
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
void* basic_memory_arena<P>::_block_mem(arena_block* block, size_t required,
                                        size_t adjustment) noexcept {
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

inline void* memory_arena::allocate(size_t size, size_t align) noexcept {
  NTF_ASSERT(_block, "Arena not initialized");
  NTF_ASSERT(size > 0, "Invalid allocation size");

  auto* block = _block;
  const size_t available = block->size - _block_offset;
  size_t adjustment = 
    impl::align_fw_adjust(impl::ptr_add(block->data, _block_offset), align);
  size_t required = size + adjustment;

  if (available < required) {
    block = _insert_block(required); // Wil waste the last few free bytes in the block
    adjustment = impl::align_fw_adjust(block->data, align);
    required = size + adjustment;
  }

  return _block_mem(block, required, adjustment);
}

inline void* memory_stack::allocate(size_t size, size_t align) noexcept {
  NTF_ASSERT(_block);
  NTF_ASSERT(size > 0, "Invalid allocation size");

  auto* block = _block;
  size_t available = block->size - _block_offset;
  size_t adjustment = impl::align_fw_adjust(impl::ptr_add(block->data, _block_offset), align);
  size_t required = size + adjustment;

  NTF_ASSERT(available > required 
             && (_used+required) <= _alloc_limit, "Memory stack limit reached");
  return _block_mem(block, required, adjustment);
}

inline void memory_stack::reset() noexcept {
  _used = 0;
  _block_offset = 0;
}

} // namespace ntf
