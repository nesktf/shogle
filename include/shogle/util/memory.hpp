#pragma once

#include <shogle/core.hpp>

namespace shogle {

struct mem_alloc_interface {
  virtual ~mem_alloc_interface() = default;
  virtual void* allocate(size_t size, size_t alignment) = 0;
  virtual void deallocate(void* ptr, size_t size) noexcept = 0;
  virtual std::pair<void*, size_t> bulk_allocate(size_t size, size_t alignment) = 0;
  virtual void bulk_deallocate(void* ptr, size_t size) noexcept = 0;
};

template<typename T>
class typed_extern_alloc {
public:
  using value_type = T;
  using pointer = T*;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  template<typename U>
  using rebind = typed_extern_alloc<U>;

public:
  typed_extern_alloc(mem_alloc_interface& alloc) noexcept : _alloc(&alloc) {}

  typed_extern_alloc(const typed_extern_alloc& alloc) noexcept = default;

  template<typename U>
  requires(!std::same_as<T, U>)
  typed_extern_alloc(const rebind<U>& other) noexcept : _alloc(other.get_mem_alloc()) {}

public:
  pointer allocate(size_type n) {
    void* ptr = _alloc->allocate(n * sizeof(value_type), alignof(value_type));
    SHOGLE_THROW_IF(!ptr, std::bad_alloc());
    return static_cast<pointer>(ptr);
  }

  void deallocate(pointer ptr, size_type n) noexcept {
    _alloc->deallocate(static_cast<void*>(ptr), n * sizeof(value_type));
  }

public:
  mem_alloc_interface& get_mem_alloc() const noexcept { return *_alloc; }

public:
  template<typename U>
  bool operator==(const rebind<U>& other) const noexcept {
    return _alloc == &other.get_mem_alloc();
  }

  template<typename U>
  bool operator!=(const rebind<U>& other) const noexcept {
    return !(*this == other);
  }

private:
  mem_alloc_interface* _alloc;
};

template<typename T>
class typed_extern_deleter {
public:
  typed_extern_deleter(mem_alloc_interface& alloc) noexcept : _alloc(&alloc) {}

public:
  void operator()(T* ptr) noexcept {
    static_assert(!std::is_void_v<T>, "Can't delete void");
    static_assert(sizeof(T) > 0, "Can't delete incomplete type");
    std::destroy_at(ptr);
    _alloc->deallocate(ptr, sizeof(T));
  }

private:
  mem_alloc_interface* _alloc;
};

class default_mem_alloc : public mem_alloc_interface {
private:
  default_mem_alloc() noexcept = default;

public:
  static default_mem_alloc& instance() noexcept;
  static size_t page_size() noexcept;
  static size_t next_page_multiple(size_t size) noexcept;

public:
  void* allocate(size_t size, size_t alignment) override;
  void deallocate(void* ptr, size_t size) noexcept override;
  std::pair<void*, size_t> bulk_allocate(size_t size, size_t alignment) override;
  void bulk_deallocate(void* ptr, size_t size) noexcept override;
};

template<typename T>
class typed_alloc {
public:
  using value_type = T;
  using pointer = T*;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  template<typename U>
  using rebind = typed_extern_alloc<U>;

public:
  typed_alloc(mem_alloc_interface& alloc) noexcept = default;

  typed_alloc(const typed_alloc& alloc) noexcept = default;

  template<typename U>
  requires(!std::same_as<T, U>)
  typed_alloc(const rebind<U>&) noexcept {};

public:
  pointer allocate(size_type n) {
    void* ptr =
      default_mem_alloc::instance().allocate(n * sizeof(value_type), alignof(value_type));
    SHOGLE_THROW_IF(!ptr, std::bad_alloc());
    return static_cast<pointer>(ptr);
  }

  void deallocate(pointer ptr, size_type n) noexcept {
    default_mem_alloc::instance().deallocate(static_cast<void*>(ptr), n * sizeof(value_type));
  }

public:
  template<typename U>
  bool operator==(const rebind<U>&) const noexcept {
    return true;
  }

  template<typename U>
  bool operator!=(const rebind<U>&) const noexcept {
    return false;
  }
};

class scratch_arena {
private:
  struct create_t {};

public:
  scratch_arena(create_t, mem_alloc_interface& alloc, void* block, size_t block_size) noexcept;

  scratch_arena(mem_alloc_interface& alloc, size_t initial_size);

  NTF_DECLARE_MOVE_ONLY(scratch_arena);

public:
  static ntf::expected<scratch_arena, std::bad_alloc> from_size(mem_alloc_interface& alloc,
                                                                size_t initial_size) noexcept;

  static ntf::expected<scratch_arena, std::bad_alloc> from_size(size_t initial_size) noexcept;

public:
  void* allocate(size_t size, size_t alignment);
  void clear() noexcept;

public:
  size_t used() const noexcept;
  size_t allocated() const noexcept;

private:
  mem_alloc_interface* _alloc;
  void* _block;
  size_t _total_used;
  size_t _allocated;
};

template<typename T>
class typed_arena_alloc {
public:
  using value_type = T;
  using pointer = T*;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  template<typename U>
  using rebind = typed_arena_alloc<U>;

public:
  typed_arena_alloc(scratch_arena& arena) noexcept : _arena(&arena) {}

  typed_arena_alloc(const scratch_arena& arena) noexcept = default;

  template<typename U>
  requires(!std::same_as<T, U>)
  typed_arena_alloc(const rebind<U>& other) noexcept : _arena(other.get_arena()) {}

public:
  pointer allocate(size_type n) {
    // scratch_arena::allocate can throw
    return static_cast<pointer>(_arena->allocate(n * sizeof(value_type), alignof(value_type)));
  }

  void deallocate(pointer ptr, size_type n) noexcept {
    NTF_UNUSED(ptr);
    NTF_UNUSED(n);
  }

public:
  scratch_arena& get_arena() const noexcept { return *_arena; }

public:
  template<typename U>
  bool operator==(const rebind<U>& other) const noexcept {
    return _arena == &other.get_mem_alloc();
  }

  template<typename U>
  bool operator!=(const rebind<U>& other) const noexcept {
    return !(*this == other);
  }

private:
  scratch_arena* _arena;
};

template<typename T>
using scratch_vec = std::vector<T, typed_alloc<T>>;

template<typename T>
static scratch_vec<T> make_scratch_vec(scratch_arena& arena) {
  return scratch_vec<T>(typed_alloc<T>{arena});
}

} // namespace shogle
