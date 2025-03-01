#pragma once

#include "./types.hpp"

namespace ntf {

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

template<typename T, typename Deleter = std::default_delete<T>>
requires(!std::is_pointer_v<T>)
class unique_array {
public:
  using value_type = T;
  using iterator = T*;
  using const_iterator = const T*;

  using uptr_type = std::unique_ptr<T[], Deleter>;

public:
  unique_array() noexcept :
    _arr{nullptr}, _sz{0} {}

  unique_array(const Deleter& del) noexcept :
    _arr{nullptr, del}, _sz{0} {}

  explicit unique_array(uptr_type arr, size_t sz) noexcept :
    _arr{std::move(arr)}, _sz{sz} {}

  unique_array(T* ptr, size_t sz) noexcept :
    _arr{ptr}, _sz{sz} {}

  unique_array(T* ptr, size_t sz, const Deleter& del) noexcept :
    _arr{ptr, del}, _sz{sz} {}

public:
  size_t size() const noexcept { return _sz; }

  value_type* get() noexcept { return _arr.get(); }
  const value_type* get() const noexcept { return _arr.get(); }

  value_type& operator[](size_t idx) {
    NTF_ASSERT(idx < _sz);
    return _arr[idx];
  }
  const value_type& operator[](size_t idx) const {
    NTF_ASSERT(idx < _sz);
    return _arr[idx];
  }

  value_type* at(size_t idx) noexcept {
    if (!valid() || idx >= _sz) {
      return nullptr;
    }
    return &_arr[idx];
  }

  const value_type* at(size_t idx) const noexcept {
    if (!valid() || idx >= _sz) {
      return nullptr;
    }
    return &_arr[idx];
  }

  bool valid() const noexcept { return _arr.get() != nullptr; }
  explicit operator bool() const noexcept { return valid(); }

  iterator begin() noexcept { return get(); }
  const_iterator begin() const noexcept { return get(); }
  const_iterator cbegin() const noexcept { return get(); }

  iterator end() noexcept { return get()+_sz; }
  const_iterator end() const noexcept { return get()+_sz; }
  const_iterator cend() const noexcept { return get()+_sz; }

public:
  void reset() noexcept {
    _arr.reset();
    _sz = 0;
  }

  void reset(T* ptr, size_t sz) noexcept {
    _arr.reset(ptr),
    _sz = sz;
  }

  [[nodiscard]] std::pair<T*, size_t> release() noexcept {
    return std::make_pair(_arr.release(), _sz);
  }

  template<typename F>
  void for_each(F&& fun) noexcept(noexcept(fun(*get()))) {
    if (!valid() || _sz == 0) {
      return;
    }
    for (auto it = begin(); it != end(); ++it) {
      fun(*it);
    }
  }

  template<typename F>
  void for_each(F&& fun) const noexcept(noexcept(fun(*get()))) {
    if (!valid() || _sz == 0) {
      return;
    }
    for (auto it = begin(); it != end(); ++it) {
      fun(*it);
    }
  }

public:
  static unique_array<T, std::default_delete<T>> from_vector(
    const std::vector<T, std::allocator<T>>& vec
  ) {
    T* arr = vec.get_allocator().allocate(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
      std::construct_at(arr+i, vec[i]);
    }
    return unique_array<T, std::default_delete<T>>{arr, vec.size()};
  }

private:
  uptr_type _arr;
  size_t _sz;

public:
  ~unique_array() = default;
  unique_array(const unique_array&) = delete;
  unique_array& operator=(const unique_array&) = delete;
  unique_array(unique_array&&) = default;
  unique_array& operator=(unique_array&&) = default;
};

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
