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

constexpr uint32 VSPAN_TOMBSTONE = std::numeric_limits<uint32>::max();
struct vec_span {
  uint32 index;
  uint32 count;

  template<typename T, typename Fun>
  void for_each(std::vector<T>& vec, Fun&& f) const {
    NTF_ASSERT(index != VSPAN_TOMBSTONE);
    NTF_ASSERT(index+count <= vec.size());
    for (uint32 i = index; i < count; ++i) {
      f(vec[i]);
    }
  }

  template<typename T, typename Fun>
  void for_each(const std::vector<T>& vec, Fun&& f) const {
    NTF_ASSERT(index != VSPAN_TOMBSTONE);
    NTF_ASSERT(index+count <= vec.size());
    for (uint32 i = index; i < count; ++i) {
      f(vec[i]);
    }
  }
};

} // namespace ntf
