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

  void* allocate(std::size_t size, std::size_t align) noexcept;
  void deallocate(void*) noexcept {}

  void reset() noexcept;
  void set_alloc(std::size_t size) noexcept;
  void decrease_alloc(std::size_t size) noexcept;

public:
  std::size_t allocated() const { return _allocated; }

private:
  void* _base{nullptr};
  std::uint64_t _max_size{0};
  std::size_t _allocated{0};

public:
  ~mem_arena() noexcept;
  mem_arena(const mem_arena&) = delete;
  mem_arena(mem_arena&&) = delete;
  mem_arena& operator=(const mem_arena&) = delete;
  mem_arena& operator=(mem_arena&&) = delete;
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
    handle(mem_pool& pool, std::size_t pos) :
      _pool(&pool), _pos(pos) {}
      
  public:
    T& operator*() {

    }

  private:
    mem_pool* _pool;
    std::size_t _pos;

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
  std::size_t allocated() const { return _arena.allocated(); }

private:
  mem_arena _arena{};
  node_type* _free_list{nullptr};
};

} // namespace ntf

#ifndef SHOGLE_STL_ARENA_INL
#include "./arena.inl"
#endif
