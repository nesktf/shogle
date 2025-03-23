#pragma once

#include "./types.hpp"
#include "../stl/allocator.hpp"

namespace ntf {

NTF_DECLARE_TAG_TYPE(uninitialized);

template<typename T>
class weak_ref {
public:
  constexpr weak_ref() noexcept : _ptr{nullptr} {}
  constexpr weak_ref(T* obj) noexcept : _ptr{obj} {}
  constexpr weak_ref(T& obj) noexcept : _ptr{std::addressof(obj)} {}

public:
  constexpr void reset() { _ptr = nullptr; }
  constexpr void reset(T& obj) { _ptr = std::addressof(obj); }

public:
  constexpr const T* operator->() const { NTF_ASSERT(_ptr); return _ptr; }
  constexpr T* operator->() { NTF_ASSERT(_ptr); return _ptr; }

  constexpr const T& operator*() const { NTF_ASSERT(_ptr); return *_ptr; }
  constexpr T& operator*() { NTF_ASSERT(_ptr); return *_ptr; }

  constexpr const T& get() const { return **this; }
  constexpr T& get() { return **this; }

  [[nodiscard]] constexpr bool valid() const { return _ptr != nullptr; }
  constexpr explicit operator bool() const { return valid(); }

private:
  T* _ptr;
};

template<typename T>
class span_view {
public:
  using const_iterator = const T*;

public:
  constexpr span_view() noexcept :
    _data{nullptr}, _size{0} {}

  constexpr span_view(const T* arr, size_t size) noexcept :
    _data{arr}, _size{size} {}

  constexpr span_view(const T& obj) noexcept :
    _data{std::addressof(obj)}, _size{1} {}

  constexpr span_view(const T* obj) noexcept :
    _data{obj}, _size{1} {}

  template<size_t N>
  constexpr span_view(const T(&arr)[N]) noexcept :
    _data{std::addressof(arr)}, _size{N} {}

  template<typename Alloc>
  constexpr span_view(const std::vector<T, Alloc>& vec) noexcept :
    _data{vec.data()}, _size{vec.size()} {}

  template<size_t N>
  constexpr span_view(const std::array<T, N>& arr) noexcept :
    _data{arr.data()}, _size{N} {}

public:
  constexpr const T& operator[](size_t idx) const {
    NTF_ASSERT(idx <= _size);
    return _data[idx];
  }

  constexpr const T* data() const noexcept { return _data; }
  [[nodiscard]] constexpr size_t size() const noexcept { return _size; }

  constexpr bool has_data() const { return _data != nullptr && _size > 0; }
  constexpr explicit operator bool() const { return has_data(); }

  constexpr const_iterator begin() const { return _data; }
  constexpr const_iterator cbegin() const { return _data; }
  constexpr const_iterator end() const { return _data+_size;}
  constexpr const_iterator cend() const { return _data+_size; }

private:
  const T* _data;
  size_t _size;
};

namespace impl {

template<typename T, typename Deleter>
class unique_array_storage {
public:
  unique_array_storage(T* arr_, size_t sz_)
  noexcept(std::is_nothrow_default_constructible_v<Deleter>) :
    arr{arr_}, sz{sz_}, del{} {}

  unique_array_storage(T* arr_, size_t sz_, const Deleter& del_)
  noexcept(std::is_nothrow_copy_constructible_v<Deleter>) :
    arr{arr_}, sz{sz_}, del{del_} {}

public:
  void reset(T* arr_ = nullptr, size_t sz_ = 0)
  noexcept(noexcept(del(arr, sz))) {
    if (arr) {
      del(arr, sz);
    }
    arr = arr_;
    sz = sz_;
  }

  Deleter& get_deleter() noexcept { return del; }
  const Deleter& get_deleter() const noexcept { return del; }

public:
  ~unique_array_storage() noexcept(noexcept(del(arr, sz))) {
    if (!arr) {
      return;
    }
    del(arr, sz);
  }

  unique_array_storage(unique_array_storage&& other)
  noexcept(std::is_nothrow_move_constructible_v<Deleter>) :
    arr{other.arr}, sz{other.sz}, del{std::move(other.del)} { other.arr = nullptr; }

  unique_array_storage& operator=(unique_array_storage&& other)
  noexcept(std::is_nothrow_move_assignable_v<Deleter>) {
    reset(other.arr, other.sz);

    del = std::move(other.del);

    other.arr = nullptr;

    return *this;
  }

  unique_array_storage(const unique_array_storage&) = delete;
  unique_array_storage& operator=(const unique_array_storage&) = delete;

public:
  T* arr;
  size_t sz;
  Deleter del;
};

template<typename T, typename Alloc>
requires(
  stateless_allocator_type<Alloc, T>
)
class unique_array_storage<T, allocator_delete<T, Alloc>> {
public:
  unique_array_storage(T* arr_, size_t sz_) noexcept :
    arr{arr_}, sz{sz_} {}

  unique_array_storage(T* arr_, size_t sz_, const allocator_delete<T, Alloc>&) noexcept :
    arr{arr_}, sz{sz_} {}

public:
  void reset(T* arr_ = nullptr, size_t sz_ = 0)
  noexcept(std::is_nothrow_destructible_v<T>) {
    if (arr) {
      allocator_delete<T, Alloc>{}(arr, sz);
    }
    arr = arr_;
    sz = sz_;
  }

  const allocator_delete<T, Alloc>& get_deleter() const noexcept { return {}; }

public:
  ~unique_array_storage() noexcept(std::is_nothrow_destructible_v<T>) {
    if (!arr) {
      return;
    }
    allocator_delete<T, Alloc>{}(arr, sz);
  }

  unique_array_storage(unique_array_storage&& other) noexcept :
    arr{other.arr}, sz{other.sz} { other.arr = nullptr; }

  unique_array_storage& operator=(unique_array_storage&& other) noexcept {
    reset(other.arr, other.sz);

    other.arr = nullptr;

    return *this;
  }

  unique_array_storage(const unique_array_storage&) = delete;
  unique_array_storage& operator=(const unique_array_storage&) = delete;

public:
  T* arr;
  size_t sz;
};

template<typename T>
class unique_array_storage<T, std::default_delete<T[]>> {
public:
  unique_array_storage(T* arr_, size_t sz_) noexcept :
    arr{arr_}, sz{sz_} {}

  unique_array_storage(T* arr_, size_t sz_,
                       const std::default_delete<T[]>&) noexcept :
    arr{arr_}, sz{sz_} {}

public:
  void reset(T* arr_ = nullptr, size_t sz_ = 0)
  noexcept(std::is_nothrow_destructible_v<T>) {
    if (arr) {
      std::default_delete<T[]>{}(arr);
    }
    arr = arr_;
    sz = sz_;
  }

  const std::default_delete<T[]>& get_deleter() const noexcept { return {}; }

public:
  ~unique_array_storage() noexcept(std::is_nothrow_destructible_v<T>) {
    if (!arr) {
      return;
    }
    // std::default_delete<T[]> calls ::operator delete[]
    // So it should ONLY be used for pointers acquired from ::operator new[]
    // NOT from std::allocator<T>::allocate;
    // use allocator_delete<T[], std::allocator<T>> instead in that case
    std::default_delete<T[]>{}(arr);
  }

  unique_array_storage(unique_array_storage&& other) noexcept :
    arr{other.arr}, sz{other.sz} { other.arr = nullptr; }

  unique_array_storage& operator=(unique_array_storage&& other) noexcept {
    reset(other.arr, other.sz);

    other.arr = nullptr;

    return *this;
  }

  unique_array_storage(const unique_array_storage&) = delete;
  unique_array_storage& operator=(const unique_array_storage&) = delete;

public:
  T* arr;
  size_t sz;
};

} // namespace impl

template<typename Deleter, typename T>
concept array_deleter_type = requires(Deleter& del, T* arr, size_t n) {
  { del(arr, n) } -> std::same_as<void>;
} || std::same_as<Deleter, std::default_delete<T[]>>;


template<typename T, array_deleter_type<T> Deleter = allocator_delete<T, std::allocator<T>>>
requires(!std::is_pointer_v<T>)
class unique_array {
public:
  using value_type = T;
  using iterator = T*;
  using const_iterator = const T*;
  using deleter_type = Deleter;

  using uptr_type = std::unique_ptr<T[], Deleter>;

public:
  unique_array()
  noexcept(std::is_nothrow_default_constructible_v<Deleter>) :
    _arr{nullptr, 0} {}

  unique_array(std::nullptr_t)
  noexcept(std::is_nothrow_default_constructible_v<Deleter>) :
    _arr{nullptr, 0} {}

  unique_array(const Deleter& del)
  noexcept(std::is_nothrow_copy_constructible_v<Deleter>) :
    _arr{nullptr, 0, del} {}

  unique_array(T* arr, size_t sz)
  noexcept(std::is_nothrow_default_constructible_v<Deleter>) :
    _arr{arr, sz} {}

  unique_array(T* arr, size_t sz, const Deleter& del)
  noexcept(std::is_nothrow_copy_constructible_v<Deleter>) :
    _arr{arr, sz, del} {}

  unique_array(uptr_type&& arr, size_t sz)
  noexcept(std::is_nothrow_copy_constructible_v<Deleter>) :
    _arr{arr.release(), sz, arr.get_deleter()} {}

public:
  size_t size() const noexcept { return _arr.sz; }

  value_type* get() noexcept { return _arr.arr; }
  const value_type* get() const noexcept { return _arr.arr; }

  bool has_data() const noexcept { return get() != nullptr; }
  explicit operator bool() const noexcept { return has_data(); }

  value_type& operator[](size_t idx) {
    NTF_ASSERT(idx < size());
    return get()[idx];
  }
  const value_type& operator[](size_t idx) const {
    NTF_ASSERT(idx < size());
    return get()[idx];
  }

  value_type* at(size_t idx) noexcept {
    if (!has_data() || idx >= size()) {
      return nullptr;
    }
    return get()+idx;
  }

  const value_type* at(size_t idx) const noexcept {
    if (!has_data() || idx >= size()) {
      return nullptr;
    }
    return get()+idx;
  }

  Deleter& get_deleter() noexcept { return _arr.get_deleter(); }
  const Deleter& get_deleter() const noexcept { return _arr.get_deleter(); }

  iterator begin() noexcept { return get(); }
  const_iterator begin() const noexcept { return get(); }
  const_iterator cbegin() const noexcept { return get(); }

  iterator end() noexcept { return get()+size(); }
  const_iterator end() const noexcept { return get()+size(); }
  const_iterator cend() const noexcept { return get()+size(); }

public:
  void reset() noexcept(noexcept(_arr.reset())) {
    _arr.reset();
  }

  void reset(T* ptr, size_t sz) noexcept(noexcept(_arr.reset(ptr, sz))) {
    _arr.reset(ptr, sz);
  }

  [[nodiscard]] std::pair<T*, size_t> release() noexcept {
    auto* ptr = get();
    _arr.arr = nullptr;
    return std::make_pair(ptr, size());
  }

  template<typename F>
  void for_each(F&& fun) noexcept(noexcept(fun(*get()))) {
    if (!has_data() || size() == 0) {
      return;
    }
    for (auto it = begin(); it != end(); ++it) {
      fun(*it);
    }
  }

  template<typename F>
  void for_each(F&& fun) const noexcept(noexcept(fun(*get()))) {
    if (!has_data() || size() == 0) {
      return;
    }
    for (auto it = begin(); it != end(); ++it) {
      fun(*it);
    }
  }

public:
  template<typename Cont>
  static auto from_container(
    Cont&& container
  ) {
    using alloc_type = std::remove_cvref_t<decltype(container.get_allocator())>;
    static_assert(allocator_type<alloc_type, T>);
    T* arr = container.get_allocator().allocate(container.size());
    size_t i = 0;
    for (auto it = container.begin(); it != container.end(); ++it) {
      std::construct_at(arr+i, std::forward<T>(*it));
      ++i;
    }
    return unique_array<T, allocator_delete<T, alloc_type>>{
      arr, 
      container.size(),
      allocator_delete<T, alloc_type>{container.get_allocator()}
    };
  }

  template<typename Cont, allocator_type<T> Alloc>
  static auto from_container(
    Cont&& container, Alloc&& alloc
  ) -> unique_array<T, allocator_delete<T, Alloc>> {
    T* arr = alloc.allocate(container.size());
    size_t i = 0;
    for (auto it = container.begin(); it != container.end(); ++it) {
      std::construct_at(arr+i, std::forward<T>(*it));
      ++i;
    }
    return {arr, container.size(), allocator_delete<T, Alloc>{alloc}};
  }

  template<allocator_type<T> Alloc = std::allocator<T>>
  static auto from_allocator(
    size_t sz, const T& copy_obj, Alloc&& alloc = {}
  ) -> unique_array<T, allocator_delete<T, Alloc>>{
    auto del = allocator_delete<T, Alloc>{alloc};
    try {
      auto* arr = alloc.allocate(sz);
      if (!arr) {
        return {del};
      }
      for (size_t i = 0; i < sz; ++i) {
        std::construct_at(arr+i, copy_obj);
      }
      return {arr, sz, del};
    } catch (...) {
      return {del};
    }
  }

  template<allocator_type<T> Alloc = std::allocator<T>>
  static auto from_allocator(
    uninitialized_t, size_t sz, Alloc&& alloc = {}
  ) -> unique_array<T, allocator_delete<T, Alloc>> {
    allocator_delete<T, Alloc> del{alloc};
    try {
      auto* arr = alloc.allocate(sz);
      if (!arr) {
        return {del};
      }
      return {arr, sz, del};
    } catch (...) {
      return {del};
    }
  }

private:
  impl::unique_array_storage<T, Deleter> _arr;

public:
  ~unique_array() noexcept = default;
  unique_array(const unique_array&) = delete;
  unique_array& operator=(const unique_array&) = delete;
  unique_array(unique_array&&) noexcept = default;
  unique_array& operator=(unique_array&&) noexcept = default;
};

NTF_DEFINE_TEMPLATE_CHECKER(unique_array);

template<typename Arr, typename T>
struct unique_array_check_t : public std::false_type {};
template<typename T, typename Deleter>
struct unique_array_check_t<unique_array<T, Deleter>, T> : public std::true_type {};
template<typename Arr, typename T>
constexpr bool unique_array_check_t_v = unique_array_check_t<Arr, T>::value;
template<typename Arr, typename T>
concept unique_array_with_type = unique_array_check_t_v<Arr, T>;

template<typename T, allocator_type<T> Alloc>
using alloc_unique_array = unique_array<T, allocator_delete<T, Alloc>>;

constexpr uint32 VSPAN_TOMBSTONE = std::numeric_limits<uint32>::max();
struct vec_span {
  uint32 index;
  uint32 count;

  template<typename Vec, typename Fun>
  void for_each(Vec& vec, Fun&& f) const {
    NTF_ASSERT(index != VSPAN_TOMBSTONE);
    NTF_ASSERT(index+count <= vec.size());
    for (uint32 i = index; i < index+count; ++i) {
      f(vec[i]);
    }
  }

  template<typename Vec, typename Fun>
  void for_each(const Vec& vec, Fun&& f) const {
    NTF_ASSERT(index != VSPAN_TOMBSTONE);
    NTF_ASSERT(index+count <= vec.size());
    for (uint32 i = index; i < index+count; ++i) {
      f(vec[i]);
    }
  }
};

} // namespace ntf
