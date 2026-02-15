#pragma once

#include <shogle/util/optional.hpp>

#include <fmt/format.h>

#include <memory>
#include <utility>
#include <vector>

namespace shogle::mem {

struct uninitialized_t {};

constexpr inline uninitialized_t uninitialized;

template<typename T>
concept bulk_memory_resource_type = requires(T res, size_t& size, size_t align, void* ptr) {
  { res.bulk_allocate(size, align) } -> std::same_as<void*>;
  { res.bulk_deallocate(ptr, size) } -> std::same_as<void>;
  { res.is_equal(std::as_const(res)) } -> std::convertible_to<bool>;
  requires noexcept(res.bulk_deallocate(ptr, size));
  requires noexcept(res.is_equal(std::as_const(res)));
};

class bulk_memory_resource {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;

private:
  struct vtbl_t {
    void* (*bulk_alloc)(void* user, size_t& size, size_t align);
    void (*bulk_dealloc)(void* user, void* ptr, size_t size) noexcept;
    bool (*check_equal)(void* a, const void* b) noexcept;
  };

  template<typename T>
  static constexpr vtbl_t vtbl_for{
    .bulk_alloc = +[](void* user, size_t& size, size_t align) -> void* {
      return (*static_cast<T*>(user)).bulk_allocate(size, align);
    },
    .bulk_dealloc = +[](void* user, void* ptr, size_t size) noexcept -> void {
      (*static_cast<T*>(user)).bulk_deallocate(ptr, size);
    },
    .check_equal = +[](void* a, const void* b) noexcept -> bool {
      return (*static_cast<T*>(a)).is_equal(*static_cast<const T*>(b));
    },
  };

public:
  template<bulk_memory_resource_type T>
  bulk_memory_resource(T& res) noexcept :
      _res(static_cast<void*>(std::addressof(res))), _vtbl(&vtbl_for<T>) {}

public:
  void* allocate(size_t& size, size_t align) const {
    void* ptr = _vtbl->bulk_alloc(_res, size, align);
    SHOGLE_THROW_IF(!ptr, ::std::bad_alloc());
    return ptr;
  }

  void deallocate(void* ptr, size_t size) const noexcept { _vtbl->bulk_dealloc(_res, ptr, size); }

  bool is_equal(const bulk_memory_resource& other) const noexcept {
    return _vtbl == other._vtbl && _vtbl->check_equal(_res, other._res);
  }

public:
  void* get_ptr() const noexcept { return _res; }

public:
  bool operator==(const bulk_memory_resource& other) const noexcept { return is_equal(other); }

  bool operator!=(const bulk_memory_resource& other) const noexcept { return !is_equal(other); }

private:
  void* _res;
  const vtbl_t* _vtbl;
};

template<typename T>
concept memory_resource_type = requires(T res, size_t& size, size_t align, void* ptr) {
  { res.allocate(size, align) } -> std::same_as<void*>;
  { res.deallocate(ptr, size) } -> std::same_as<void>;
  { res.is_equal(std::as_const(res)) } -> std::convertible_to<bool>;
  requires noexcept(res.deallocate(ptr, size));
  requires noexcept(res.is_equal(std::as_const(res)));
};

class memory_resource {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;

public:
  template<typename T>
  class allocator;

  template<typename T>
  class deleter;

private:
  struct vtbl_t {
    void* (*alloc)(void* user, size_t& size, size_t align);
    void (*dealloc)(void* user, void* ptr, size_t size) noexcept;
    bool (*check_equal)(void* a, const void* b) noexcept;
  };

  template<typename T>
  static constexpr vtbl_t vtbl_for{
    .alloc = +[](void* user, size_t& size, size_t align) -> void* {
      return (*static_cast<T*>(user)).bulk_allocate(size, align);
    },
    .dealloc = +[](void* user, void* ptr, size_t size) noexcept -> void {
      (*static_cast<T*>(user)).bulk_deallocate(ptr, size);
    },
    .equal = +[](void* a, const void* b) noexcept -> bool {
      return (*static_cast<T*>(a)).is_equal(*static_cast<const T*>(b));
    },
  };

public:
  template<memory_resource_type T>
  memory_resource(T& res) noexcept :
      _res(static_cast<void*>(std::addressof(res))), _vtbl(&vtbl_for<T>) {}

public:
  void* allocate(size_t& size, size_t align) const {
    void* ptr = _vtbl->alloc(_res, size, align);
    SHOGLE_THROW_IF(!ptr, ::std::bad_alloc());
    return ptr;
  }

  void deallocate(void* ptr, size_t size) const noexcept { _vtbl->dealloc(_res, ptr, size); }

  bool is_equal(const memory_resource& other) const noexcept {
    return _vtbl == other._vtbl && _vtbl->check_equal(_res, other._res);
  }

public:
  void* get_ptr() const noexcept { return _res; }

public:
  bool operator==(const memory_resource& other) const noexcept { return is_equal(other); }

  bool operator!=(const memory_resource& other) const noexcept { return !is_equal(other); }

public:
  void* _res;
  const vtbl_t* _vtbl;
};

template<typename T>
class memory_resource::allocator {
public:
  using value_type = T;
  using pointer = T*;
  using size_type = memory_resource::size_type;
  using difference_type = memory_resource::difference_type;

  template<typename U>
  using rebind = allocator<U>;

public:
  allocator(const memory_resource& res) noexcept : _res(res) {}

  allocator(const allocator&) noexcept = default;

  template<typename U>
  requires(!std::same_as<T, U>)
  allocator(const rebind<U>& other) noexcept : _res(other.get_resource()) {}

public:
  pointer allocate(size_type n) {
    size_type size = n * sizeof(value_type);
    return static_cast<pointer>(_res.allocate(size, alignof(value_type)));
  }

  void deallocate(pointer ptr, size_type n) noexcept {
    _res.deallocate(static_cast<void*>(ptr), n * sizeof(value_type));
  }

public:
  memory_resource get_resource() const noexcept { return _res; }

public:
  template<typename U>
  bool operator==(const rebind<U>& other) const noexcept {
    return _res == other._res;
  }

  template<typename U>
  bool operator!=(const rebind<U>& other) const noexcept {
    return _res != other._res;
  }

private:
  memory_resource _res;
};

template<typename T>
class memory_resource::deleter {
public:
  using value_type = T;
  using pointer = T*;
  using size_type = memory_resource::size_type;
  using difference_type = memory_resource::difference_type;

public:
  template<typename U>
  using rebind = deleter<U>;

public:
  deleter(const memory_resource& res) noexcept : _res(res) {}

  deleter(const deleter&) noexcept = default;

  template<typename U>
  requires(!std::same_as<T, U>)
  constexpr deleter(const rebind<U>& other) noexcept : _res(other.get_resource()) {}

public:
  template<typename U = T>
  requires(std::convertible_to<T*, U*>)
  void operator()(U* ptr) noexcept {
    static_assert(!std::is_void_v<T>, "Can't delete incomplete type");
    static_assert(sizeof(T) > 0, "Can't delete incomplete type");
    static_assert(std::is_nothrow_destructible_v<T>, "Can't delete throwing type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(static_cast<T*>(ptr));
    }
    _res.deallocate(static_cast<void*>(ptr), sizeof(T));
  }

  template<typename U = T>
  requires(std::convertible_to<T*, U*>)
  void operator()(U* ptr, size_type n) noexcept {
    static_assert(!std::is_void_v<T>, "Can't delete incomplete type");
    static_assert(sizeof(T) > 0, "Can't delete incomplete type");
    static_assert(std::is_nothrow_destructible_v<T>, "Can't delete throwing type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_n(static_cast<T*>(ptr), n);
    }
    _res.deallocate(static_cast<void*>(ptr), n * sizeof(T));
  }

public:
  template<typename U>
  bool operator==(const rebind<U>& other) noexcept {
    return _res == other._res;
  }

  template<typename U>
  bool operator!=(const rebind<U>& other) noexcept {
    return _res != other._res;
  }

public:
  memory_resource get_resource() const noexcept { return _res; }

private:
  memory_resource _res;
};

constexpr inline size_t align_fw_adjust(void* ptr, size_t align) noexcept {
  return align - (std::bit_cast<uintptr_t>(ptr) & (align - 1u));
}

constexpr inline void* ptr_add(void* p, uintptr_t sz) noexcept {
  return std::bit_cast<void*>(std::bit_cast<uintptr_t>(p) + sz);
}

size_t system_page_size() noexcept;

class scratch_arena {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;

public:
  template<typename T>
  class allocator {
  public:
    using value_type = T;
    using pointer = T*;
    using size_type = scratch_arena::size_type;
    using difference_type = scratch_arena::difference_type;

    template<typename U>
    using rebind = allocator<U>;

  public:
    constexpr allocator(scratch_arena& arena) noexcept : _arena(&arena) {}

    constexpr allocator(const allocator& arena) noexcept = default;

    template<typename U>
    requires(!std::same_as<T, U>)
    constexpr allocator(const rebind<U>& other) noexcept : _arena(other.get_arena()) {}

  public:
    constexpr pointer allocate(size_type n) {
      return static_cast<pointer>(_arena->allocate(n * sizeof(value_type), alignof(value_type)));
    }

    constexpr void deallocate(pointer ptr, size_type n) noexcept {
      SHOGLE_UNUSED(ptr);
      SHOGLE_UNUSED(n);
    }

  public:
    constexpr scratch_arena& get_arena() const noexcept { return *_arena; }

  public:
    template<typename U>
    constexpr bool operator==(const rebind<U>& other) const noexcept {
      return _arena->is_equal(other.get_arena());
    }

    template<typename U>
    constexpr bool operator!=(const rebind<U>& other) const noexcept {
      return !(*this == other);
    }

  private:
    scratch_arena* _arena;
  };

  template<typename T>
  class deleter {
  public:
    using value_type = T;
    using pointer = T*;
    using size_type = scratch_arena::size_type;
    using difference_type = scratch_arena::difference_type;

  public:
    template<typename U>
    using rebind = deleter<U>;

  public:
    constexpr deleter(scratch_arena& arena) noexcept : _arena(&arena) {}

    constexpr deleter(const deleter& deleter) noexcept = default;

    template<typename U>
    requires(!std::same_as<T, U>)
    constexpr deleter(const rebind<U>& other) noexcept : _arena(other.get_arena()) {}

  public:
    template<typename U = T>
    requires(std::convertible_to<T*, U*>)
    constexpr void operator()(U* ptr) noexcept {
      static_assert(!std::is_void_v<T>, "Can't delete incomplete type");
      static_assert(sizeof(T) > 0, "Can't delete incomplete type");
      static_assert(std::is_nothrow_destructible_v<T>, "Can't delete throwing type");
      if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy_at(static_cast<T*>(ptr));
      }
    }

    template<typename U = T>
    requires(std::convertible_to<T*, U*>)
    constexpr void operator()(U* ptr, size_type n) noexcept {
      static_assert(!std::is_void_v<T>, "Can't delete incomplete type");
      static_assert(sizeof(T) > 0, "Can't delete incomplete type");
      static_assert(std::is_nothrow_destructible_v<T>, "Can't delete throwing type");
      if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy_n(static_cast<T*>(ptr), n);
      }
    }

  public:
    template<typename U>
    constexpr bool operator==(const rebind<U>& other) noexcept {
      return _arena->is_equal(other.get_arena());
    }

    template<typename U>
    constexpr bool operator!=(const rebind<U>& other) noexcept {
      return !(*this == other);
    }

  public:
    constexpr scratch_arena& get_arena() const noexcept { return *_arena; }

  private:
    scratch_arena* _arena;
  };

private:
  struct create_t {};

public:
  scratch_arena(create_t, void* data, size_type block_size) noexcept;
  scratch_arena(size_type initial_size);
  explicit scratch_arena(size_type initial_size, const bulk_memory_resource& res);

  template<bulk_memory_resource_type T>
  scratch_arena(size_type initial_size, T& res) :
      scratch_arena(initial_size, bulk_memory_resource(res)) {}

public:
  scratch_arena(scratch_arena&& other) noexcept;
  scratch_arena(const scratch_arena& other) = delete;
  ~scratch_arena() noexcept;

public:
  scratch_arena& operator=(scratch_arena&& other) noexcept;
  scratch_arena& operator=(const scratch_arena& other) = delete;

public:
  static optional<scratch_arena> with_initial_size(size_type initial_size) noexcept;

  static optional<scratch_arena> with_memory_resource(size_type initial_size,
                                                      const bulk_memory_resource& res) noexcept;

  template<bulk_memory_resource_type T>
  static optional<scratch_arena> with_memory_resource(size_type initial_size, T& res) noexcept {
    return with_memory_resource(initial_size, bulk_memory_resource(res));
  }

public:
  template<typename T, typename... Args>
  T* construct(Args&&... args) {
    T* ptr = static_cast<T*>(allocate(sizeof(T), alignof(T)));
    SHOGLE_THROW_IF(!ptr, std::bad_alloc());
    new (ptr) T(std::forward<Args>(args)...);
    return ptr;
  }

  template<typename T>
  requires(std::copy_constructible<T>)
  T* construct_n(size_type n, const T& copy) {
    T* ptr = static_cast<T*>(allocate(n * sizeof(T), alignof(T)));
    SHOGLE_THROW_IF(!ptr, std::bad_alloc());
    for (size_type i = 0; i < n; ++i) {
      new (ptr + i) T(copy);
    }
    return ptr;
  }

  template<typename T>
  requires(std::is_default_constructible_v<T>)
  T* construct_n(size_type n) {
    T* ptr = static_cast<T*>(allocate(n * sizeof(T), alignof(T)));
    SHOGLE_THROW_IF(!ptr, std::bad_alloc());
    for (size_type i = 0; i < n; ++i) {
      new (ptr + i) T();
    }
    return ptr;
  }

  template<typename T>
  requires(std::is_trivially_constructible_v<T>)
  T* construct_n(uninitialized_t, size_type n) {
    T* ptr = static_cast<T*>(allocate(n * sizeof(T), alignof(T)));
    SHOGLE_THROW_IF(!ptr, std::bad_alloc());
    return ptr;
  }

  template<typename T>
  void destroy(T* ptr) noexcept(std::is_nothrow_destructible_v<T>) {
    std::destroy_at(ptr);
    deallocate(static_cast<void*>(ptr), sizeof(T));
  }

  template<typename T>
  void destroy_n(T* ptr, size_type n) noexcept(std::is_nothrow_destructible_v<T>) {
    std::destroy_n(ptr, n);
    deallocate(static_cast<void*>(ptr), n * sizeof(T));
  }

public:
  void* allocate(size_type size, size_type alignment);

  void deallocate(void* ptr, size_type size) noexcept {
    SHOGLE_UNUSED(ptr);
    SHOGLE_UNUSED(size);
  }

  bool is_equal(const scratch_arena& other) const noexcept;

  void clear() noexcept;

public:
  size_type used() const noexcept { return _used; }

  size_type allocated() const noexcept { return _allocated; }

private:
  void* _data;
  size_type _used;
  size_type _allocated;
};

} // namespace shogle::mem

namespace shogle {

namespace impl {

template<typename T, typename Deleter>
struct unique_array_del : private Deleter {
  unique_array_del() noexcept(std::is_nothrow_default_constructible_v<Deleter>) : Deleter() {}

  unique_array_del(const Deleter& del) noexcept(std::is_nothrow_copy_constructible_v<Deleter>) :
      Deleter(del) {}

  void delete_array(T* arr,
                    typename Deleter::size_type n) noexcept(std::is_nothrow_destructible_v<T>) {
    Deleter::operator()(arr, n);
  }

  Deleter& get_deleter() noexcept { return static_cast<Deleter&>(*this); }

  const Deleter& get_deleter() const noexcept { return static_cast<const Deleter&>(*this); }
};

} // namespace impl

template<typename T>
class default_array_delete {
public:
  using value_type = T;
  using pointer = T*;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

public:
  template<typename U>
  using rebind = default_array_delete<U>;

public:
  constexpr default_array_delete() noexcept = default;
  constexpr default_array_delete(const default_array_delete&) noexcept = default;

  template<typename U>
  requires(!std::same_as<T, U>)
  constexpr default_array_delete(const rebind<U>&) noexcept {}

public:
  template<typename U = T>
  requires(std::convertible_to<T*, U*>)
  constexpr void operator()(U* ptr) noexcept {
    static_assert(!std::is_void_v<T>, "Can't delete incomplete type");
    static_assert(sizeof(T) > 0, "Can't delete incomplete type");
    static_assert(std::is_nothrow_destructible_v<T>, "Can't delete throwing type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(ptr);
    }
    std::allocator<T>().deallocate(ptr, 1);
  }

  template<typename U = T>
  requires(std::convertible_to<T*, U*>)
  constexpr void operator()(U* ptr, size_type n) noexcept {
    static_assert(!std::is_void_v<T>, "Can't delete incomplete type");
    static_assert(sizeof(T) > 0, "Can't delete incomplete type");
    static_assert(std::is_nothrow_destructible_v<T>, "Can't delete throwing type");
    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_n(ptr, n);
    }
    std::allocator<T>().deallocate(ptr, n);
  }

public:
  template<typename U>
  constexpr bool operator==(const rebind<U>&) noexcept {
    return true;
  }

  template<typename U>
  constexpr bool operator!=(const rebind<U>&) noexcept {
    return false;
  }
};

template<typename T, typename Deleter = ::shogle::default_array_delete<T>>
requires(!std::is_reference_v<T>)
class unique_array : private impl::unique_array_del<T, Deleter> {
public:
  using value_type = T;
  using deleter_type = Deleter;
  using size_type = size_t;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = pointer;
  using const_iterator = const_pointer;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  using del_base = impl::unique_array_del<T, Deleter>;

public:
  unique_array() noexcept(std::is_nothrow_default_constructible_v<Deleter>) :
      del_base(), _arr{nullptr}, _sz{0u} {}

  unique_array(std::nullptr_t) noexcept(std::is_nothrow_default_constructible_v<Deleter>) :
      del_base(), _arr{nullptr}, _sz{0u} {}

  explicit unique_array(const Deleter& del) noexcept(
    std::is_nothrow_copy_constructible_v<Deleter>) : del_base(del), _arr{nullptr}, _sz{0u} {}

  unique_array(pointer arr,
               size_type n) noexcept(std::is_nothrow_default_constructible_v<Deleter>) :
      del_base(), _arr{arr}, _sz{n} {}

  unique_array(pointer arr, size_type n,
               const Deleter& del) noexcept(std::is_nothrow_copy_constructible_v<Deleter>) :
      del_base(del), _arr{arr}, _sz{n} {}

  unique_array(unique_array&& other) noexcept(std::is_nothrow_move_constructible_v<Deleter>) :
      del_base(other.get_deleter()), _arr{other._arr}, _sz{other._sz} {
    other._arr = 0u;
  }

  unique_array(const unique_array&) = delete;

  ~unique_array() noexcept(std::is_nothrow_destructible_v<T>) {
    if (!empty()) {
      del_base::delete_array(_arr, _sz);
    }
  }

public:
  unique_array& assign(pointer arr, size_type size) noexcept {
    if (!empty()) {
      del_base::delete_array(_arr, _sz);
    }

    _arr = arr;
    _sz = size;

    return *this;
  }

  void reset() noexcept { assign(nullptr, 0u); }

  [[nodiscard]] std::pair<pointer, size_type> release() noexcept {
    pointer ptr = get();
    size_type sz = size();
    _sz = 0;
    _arr = nullptr;
    return std::make_pair(ptr, sz);
  }

public:
  unique_array& operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  unique_array&
  operator=(unique_array&& other) noexcept(std::is_nothrow_move_assignable_v<Deleter>) {
    if (!empty()) {
      del_base::delete_array(_arr, _sz);
    }

    del_base::operator=(static_cast<del_base&&>(other));
    _arr = std::move(other._arr);
    _sz = std::move(other._sz);

    other._arr = nullptr;

    return *this;
  }

  unique_array& operator=(const unique_array&) = delete;

  value_type& operator[](size_type idx) {
    SHOGLE_ASSERT(idx < size());
    return get()[idx];
  }

  const value_type& operator[](size_type idx) const {
    SHOGLE_ASSERT(idx < size());
    return get()[idx];
  }

  value_type& at(size_type idx) {
    SHOGLE_THROW_IF(idx >= size(), std::out_of_range(fmt::format("Index {} out of range", idx)));
    return _arr[idx];
  }

  const value_type& at(size_type idx) const {
    SHOGLE_THROW_IF(idx >= size(), std::out_of_range(fmt::format("Index {} out of range", idx)));
    return _arr[idx];
  }

public:
  size_type size() const noexcept { return _sz; }

  pointer get() noexcept { return _arr; }

  const_pointer get() const noexcept { return _arr; }

  pointer data() noexcept { return _arr; }

  const_pointer data() const noexcept { return _arr; }

  bool empty() const noexcept { return _arr == nullptr; }

  Deleter& get_deleter() noexcept { return del_base::get_deleter(); }

  const Deleter& get_deleter() const noexcept { del_base::get_deleter(); }

public:
  explicit operator bool() const noexcept { return !empty(); }

public:
  iterator begin() noexcept { return get(); }

  const_iterator begin() const noexcept { return get(); }

  const_iterator cbegin() const noexcept { return get(); }

  iterator end() noexcept { return get() + size(); }

  const_iterator end() const noexcept { return get() + size(); }

  const_iterator cend() const noexcept { return get() + size(); }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

  const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

  const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

  const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

  const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

private:
  pointer _arr;
  size_t _sz;
};

template<typename T>
requires(std::is_default_constructible_v<T>)
unique_array<T> make_array(size_t n) {
  T* ptr = std::allocator<T>().allocate(n);
  for (size_t i = 0; i < n; ++i) {
    new (ptr + i) T();
  }
  return unique_array<T>(ptr, n);
}

template<typename T>
requires(std::copy_constructible<T>)
unique_array<T> make_array(size_t n, const T& copy) {
  T* ptr = std::allocator<T>().allocate(n);
  for (size_t i = 0; i < n; ++i) {
    new (ptr + i) T(copy);
  }
  return unique_array<T>(ptr, n);
}

template<typename T>
requires(std::is_trivially_constructible_v<T>)
unique_array<T> make_array(mem::uninitialized_t, size_t n) {
  return unique_array<T>(std::allocator<T>().allocate(n), n);
}

template<typename T>
using scratch_vec = std::vector<T, mem::scratch_arena::allocator<T>>;

template<typename T>
scratch_vec<T> make_scratch_vec(mem::scratch_arena& arena) {
  return scratch_vec<T>(mem::scratch_arena::allocator<T>{arena});
}

} // namespace shogle
