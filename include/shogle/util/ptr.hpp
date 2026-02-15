#pragma once

#include <shogle/core.hpp>

#include <fmt/format.h>

#include <functional>
#include <limits>
#include <memory>
#include <type_traits>

namespace shogle {

template<typename T>
requires(!std::is_void_v<T> && !std::is_reference_v<T>)
class ptr_view {
public:
  using element_type = T;
  using value_type = std::remove_cvref_t<T>;

  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

public:
  constexpr ptr_view() noexcept = default;

  constexpr ptr_view(std::nullptr_t) noexcept : _ptr{nullptr} {}

  constexpr ptr_view(pointer ptr) noexcept : _ptr{ptr} {}

  template<typename U>
  requires(std::is_convertible_v<U*, T*>)
  constexpr ptr_view(U& obj) noexcept : _ptr{std::addressof(obj)} {}

  template<typename U>
  requires(std::is_convertible_v<U*, T*> && !std::same_as<U, T>)
  constexpr ptr_view(U* ptr) noexcept : _ptr(ptr) {}

  template<typename U>
  requires(std::is_convertible_v<U*, T*>)
  constexpr ptr_view(const ptr_view<U>& other) noexcept : _ptr{other.data()} {}

  constexpr ptr_view(const ptr_view&) noexcept = default;
  constexpr ptr_view(ptr_view&&) noexcept = default;

  constexpr ~ptr_view() noexcept = default;

public:
  constexpr reference get() const {
    SHOGLE_ASSERT(_ptr, "Invalid pointer");
    return *_ptr;
  }

  constexpr pointer ptr() const noexcept { return _ptr; }

  constexpr pointer data() const noexcept { return _ptr; }

public:
  [[nodiscard]] constexpr bool empty() const { return _ptr == nullptr; }

  constexpr explicit operator bool() const { return !empty(); }

  constexpr pointer operator->() const {
    SHOGLE_ASSERT(_ptr, "Invalid pointer");
    return _ptr;
  }

  constexpr reference operator*() const {
    SHOGLE_ASSERT(_ptr, "Invalid pointer");
    return *_ptr;
  }

  constexpr ptr_view& operator=(std::nullptr_t) noexcept {
    _ptr = nullptr;
    return *this;
  }

  constexpr ptr_view& operator=(pointer ptr) noexcept {
    _ptr = ptr;
    return *this;
  }

  template<typename U>
  requires(std::is_convertible_v<U*, T*> && !std::same_as<U, T>)
  constexpr ptr_view& operator=(U* ptr) {
    _ptr = ptr;
    return *this;
  }

  template<typename U>
  requires(std::is_convertible_v<U*, T*>)
  constexpr ptr_view& operator=(U& obj) noexcept {
    _ptr = std::addressof(obj);
    return *this;
  }

  template<typename U>
  requires(std::is_convertible_v<U*, T*>)
  constexpr ptr_view& operator=(const ptr_view<U>& other) noexcept {
    _ptr = other.data();
    return *this;
  }

  constexpr ptr_view& operator=(const ptr_view&) noexcept = default;
  constexpr ptr_view& operator=(ptr_view&&) noexcept = default;

public:
  operator reference() const { return get(); }

private:
  T* _ptr;
};

template<typename T>
requires(!std::is_void_v<T> && !std::is_reference_v<T>)
class ref_view {
public:
  using element_type = T;
  using value_type = std::remove_cvref_t<T>;

  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

public:
  constexpr ref_view(reference obj) noexcept : _ptr(std::addressof(obj)) {}

  template<typename U>
  requires(std::is_convertible_v<U*, T*>)
  constexpr ref_view(U& obj) noexcept : _ptr(std::addressof(obj)) {}

  constexpr explicit ref_view(pointer ptr) : _ptr(ptr) {
    SHOGLE_THROW_IF(!_ptr, std::runtime_error("Assigning nullptr to ref_view"));
  }

  template<typename U>
  requires(std::is_convertible_v<U*, T*> && !std::same_as<U, T>)
  constexpr ref_view(U* ptr) : _ptr(ptr) {
    SHOGLE_THROW_IF(!_ptr, std::runtime_error("Assigning nullptr to ref_view"));
  }

  template<typename U>
  requires(std::is_convertible_v<U*, T*>)
  constexpr ref_view(const ref_view<U>& other) noexcept : _ptr{other.data()} {}

  constexpr ref_view(const ref_view&) noexcept = default;
  constexpr ref_view(ref_view&&) noexcept = default;

  constexpr ~ref_view() noexcept = default;

public:
  constexpr reference get() const noexcept { return *_ptr; }

  constexpr pointer ptr() const noexcept { return _ptr; }

  constexpr pointer data() const noexcept { return _ptr; }

public:
  constexpr pointer operator->() const noexcept { return _ptr; }

  constexpr reference operator*() const noexcept { return *_ptr; }

  constexpr ref_view& operator=(pointer ptr) {
    SHOGLE_THROW_IF(!ptr, std::runtime_error("Assigning nullptr to ref_view"));
    _ptr = ptr;
    return *this;
  }

  template<typename U>
  requires(std::is_convertible_v<U*, T*> && !std::same_as<U, T>)
  constexpr ref_view& operator=(U* ptr) {
    SHOGLE_THROW_IF(!ptr, std::runtime_error("Assigning nullptr to ref_view"));
    _ptr = ptr;
    return *this;
  }

  template<typename U>
  requires(std::is_convertible_v<U*, T*>)
  constexpr ref_view& operator=(U& obj) noexcept {
    _ptr = std::addressof(obj);
    return *this;
  }

  template<typename U>
  requires(std::is_convertible_v<U*, T*>)
  constexpr ref_view& operator=(const ref_view<U>& other) noexcept {
    _ptr = other.ptr();
    return *this;
  }

  constexpr ref_view& operator=(const ref_view&) noexcept = default;
  constexpr ref_view& operator=(ref_view&&) noexcept = default;

public:
  operator reference() const noexcept { return get(); }

private:
  T* _ptr;
};

constexpr size_t dynamic_extent = std::numeric_limits<size_t>::max();

namespace impl {

template<size_t SpanExtent>
struct span_extent {
  constexpr span_extent(size_t ext) { SHOGLE_ASSERT(ext == SpanExtent); }

  constexpr size_t get_extent() const noexcept { return SpanExtent; }

  constexpr void assign_extent(size_t) noexcept {}
};

template<>
struct span_extent<dynamic_extent> {
  constexpr span_extent() noexcept = default;

  constexpr span_extent(size_t ext) noexcept : _ext{ext} {}

  constexpr size_t get_extent() const noexcept { return _ext; }

  constexpr void assign_extent(size_t ext) noexcept { _ext = ext; }

private:
  size_t _ext;
};

} // namespace impl

template<typename T, size_t SpanExtent = dynamic_extent>
class span : public impl::span_extent<SpanExtent> {
public:
  using element_type = T;
  using value_type = std::remove_cv_t<T>;

  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

  using iterator = pointer;
  using const_iterator = const_pointer;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using size_type = size_t;
  using difference_type = ptrdiff_t;

  static constexpr size_type extent = dynamic_extent;

public:
  constexpr span() noexcept
  requires(extent == dynamic_extent)
  = default;

  constexpr explicit span(reference obj) noexcept
  requires(extent == 1u || extent == dynamic_extent)
      : impl::span_extent<SpanExtent>{1u}, _data{std::addressof(obj)} {}

  template<typename It>
  requires(std::contiguous_iterator<It>)
  explicit(extent != dynamic_extent) constexpr span(It first, size_type count)
  requires(std::is_convertible_v<std::remove_reference_t<std::iter_reference_t<It>> (*)[],
                                 element_type (*)[]>)
      : impl::span_extent<SpanExtent>{count}, _data{std::to_address(first)} {}

  template<typename It, typename End>
  requires(std::contiguous_iterator<It> && std::sized_sentinel_for<End, It>)
  explicit(extent != dynamic_extent) constexpr span(It first, End last)
  requires(std::is_convertible_v<std::remove_reference_t<std::iter_reference_t<It>> (*)[],
                                 element_type (*)[]>)
      : impl::span_extent<SpanExtent>{last - first}, _data{std::to_address(first)} {}

  template<size_t N>
  constexpr span(std::type_identity_t<element_type> (&arr)[N]) noexcept
  requires(std::is_convertible_v<std::remove_pointer_t<decltype(std::data(arr))> (*)[],
                                 element_type (*)[]> &&
           (extent == dynamic_extent || extent == N))
      : impl::span_extent<SpanExtent>{N}, _data{std::data(arr)} {}

  template<typename U, size_t N>
  constexpr span(std::array<U, N>& arr) noexcept
  requires(std::is_convertible_v<std::remove_pointer_t<decltype(std::data(arr))> (*)[],
                                 element_type (*)[]> &&
           (extent == dynamic_extent || extent == N))
      : impl::span_extent<SpanExtent>{N}, _data{std::data(arr)} {}

  template<typename U, size_t N>
  constexpr span(const std::array<U, N>& arr) noexcept
  requires(std::is_convertible_v<std::remove_pointer_t<decltype(std::data(arr))> (*)[],
                                 element_type (*)[]> &&
           (extent == dynamic_extent || extent == N))
      : impl::span_extent<SpanExtent>{N}, _data{std::data(arr)} {}

  explicit(extent != dynamic_extent) constexpr span(std::initializer_list<value_type> il) :
      impl::span_extent<SpanExtent>{il.size()}, _data{il.begin()} {}

  template<typename U, size_t N>
  explicit(extent != dynamic_extent &&
           N == dynamic_extent) constexpr span(const span<U, N>& src) noexcept
  requires(N != extent && std::is_convertible_v<U (*)[], element_type (*)[]> &&
           (extent == dynamic_extent || N == dynamic_extent || extent == N))
      : impl::span_extent<SpanExtent>{N}, _data{src.data()} {}

  template<typename U>
  constexpr span(const span<U, dynamic_extent>& src) noexcept
  requires(std::is_convertible_v<U (*)[], element_type (*)[]> && extent == dynamic_extent)
      : impl::span_extent<dynamic_extent>{src.size()}, _data{src.data()} {}

  constexpr span(const span&) noexcept = default;
  constexpr span(span&&) noexcept = default;

  constexpr ~span() noexcept = default;

public:
  template<size_t N>
  constexpr span<element_type, N> first() const
  requires(N <= extent || extent == dynamic_extent)
  {
    if constexpr (extent == dynamic_extent) {
      SHOGLE_ASSERT(N < size());
    }
    return {data(), N};
  }

  constexpr span<element_type, dynamic_extent> first(size_type count) const {
    SHOGLE_ASSERT(count < size());
    return {data(), count};
  }

  template<size_t N>
  constexpr span<element_type, N> last() const
  requires(N <= extent || extent == dynamic_extent)
  {
    if constexpr (extent == dynamic_extent) {
      SHOGLE_ASSERT(N < size());
    }
    return {data() + (size() - N), N};
  }

  constexpr span<element_type, dynamic_extent> last(size_type count) const {
    SHOGLE_ASSERT(count < size());
    return {data() + (size() - count), count};
  }

public:
  constexpr iterator begin() const noexcept { return _data; }

  constexpr const_iterator cbegin() const noexcept { return _data; }

  constexpr iterator end() const noexcept { return _data + size(); }

  constexpr const_iterator cend() const noexcept { return _data + size(); }

  constexpr reverse_iterator rbegin() const noexcept { return {_data + size() - 1}; }

  constexpr const_reverse_iterator crbegin() const noexcept { return {_data + size() - 1}; }

  constexpr reverse_iterator rend() const noexcept { return {_data - 1}; }

  constexpr const_reverse_iterator crend() const noexcept { return {_data - 1}; }

public:
  constexpr pointer data() const noexcept { return _data; }

  constexpr size_type size() const noexcept { return impl::span_extent<SpanExtent>::get_extent(); }

  constexpr size_type size_bytes() const { return size() * sizeof(T); }

  constexpr reference front() const { return *begin(); }

  constexpr reference back() const { return *(end() - 1); }

  constexpr bool empty() const noexcept { return size() == 0u; }

  constexpr reference at(size_type idx) const {
    SHOGLE_THROW_IF(idx >= size(), std::out_of_range(fmt::format("Index {} out of range", idx)));
    return _data[idx];
  }

  constexpr pointer at_opt(size_type idx) noexcept {
    return idx >= size() ? nullptr : data() + idx;
  }

public:
  constexpr explicit operator bool() const noexcept { return !empty(); }

  constexpr reference operator[](size_type idx) const {
    SHOGLE_ASSERT(idx < size());
    return _data[idx];
  }

  constexpr span& operator=(const span& other) noexcept = default;
  constexpr span& operator=(span&& other) noexcept = default;

  template<typename U, size_t N>
  requires(std::is_convertible_v<U (*)[], element_type (*)[]> &&
           (extent == dynamic_extent || N == dynamic_extent || extent == N))
  constexpr span& operator=(const span<U, N>& other) noexcept {
    impl::span_extent<SpanExtent>::assign_extent(other.size());
    _data = other.data();
    return *this;
  }

private:
  pointer _data;
};

template<typename Signature>
class function_view;

template<typename Ret, typename... Args>
class function_view<Ret(Args...)> {
private:
  template<typename T>
  static constexpr Ret _invoke_for(void* obj, Args... args) {
    if constexpr (std::is_void_v<Ret>) {
      std::invoke(*static_cast<T*>(obj), std::forward<Args>(args)...);
    } else {
      return std::invoke(*static_cast<T*>(obj), std::forward<Args>(args)...);
    }
  }

public:
  using signature = Ret(Args...);
  using return_type = Ret;

public:
  constexpr function_view() noexcept : _data{nullptr}, _invoke_ptr{nullptr} {}

  explicit constexpr function_view(std::nullptr_t) noexcept :
      _data{nullptr}, _invoke_ptr{nullptr} {}

  explicit constexpr function_view(Ret (*fun)(Args...)) noexcept :
      _data{nullptr}, _invoke_ptr{fun} {}

  template<typename T>
  requires(std::is_invocable_r_v<Ret, T, Args...> && !std::is_same_v<function_view, T>)
  constexpr function_view(T& functor) noexcept :
      _data{static_cast<void*>(std::addressof(functor))}, _invoke_functor{&_invoke_for<T>} {}

  template<typename T>
  requires(std::is_invocable_r_v<Ret, T, Args...> && !std::is_same_v<function_view, T>)
  constexpr function_view(T* functor) noexcept :
      _data{static_cast<void*>(functor)}, _invoke_functor{&_invoke_for<T>} {}

public:
  constexpr ~function_view() noexcept = default;
  constexpr function_view(const function_view&) noexcept = default;
  constexpr function_view(function_view&&) noexcept = default;

public:
  constexpr Ret operator()(Args... args) const {
    if (_data) {
      SHOGLE_ASSERT(_invoke_functor);
      if constexpr (std::is_void_v<Ret>) {
        std::invoke(_invoke_functor, _data, std::forward<Args>(args)...);
      } else {
        return std::invoke(_invoke_functor, _data, std::forward<Args>(args)...);
      }
    } else {
      SHOGLE_ASSERT(_invoke_ptr);
      if constexpr (std::is_void_v<Ret>) {
        std::invoke(_invoke_ptr, std::forward<Args>(args)...);
      } else {
        return std::invoke(_invoke_ptr, std::forward<Args>(args)...);
      }
    }
  }

  constexpr function_view& operator=(std::nullptr_t) noexcept {
    _data = nullptr;
    _invoke_ptr = nullptr;
    return *this;
  }

  constexpr function_view& operator=(Ret (*fun)(Args...)) noexcept {
    _data = nullptr;
    _invoke_ptr = fun;
    return *this;
  }

  template<typename T>
  requires(std::is_invocable_r_v<Ret, T, Args...> && !std::is_same_v<function_view, T>)
  constexpr function_view& operator=(T& functor) noexcept {
    _data = std::addressof(functor);
    _invoke_functor = &_invoke_for<T>;
    return *this;
  }

  constexpr function_view& operator=(const function_view&) noexcept = default;
  constexpr function_view& operator=(function_view&&) noexcept = default;

public:
  constexpr bool is_empty() const { return !(_data && _invoke_functor) || _invoke_ptr == nullptr; }

  constexpr explicit operator bool() const { return !is_empty(); }

private:
  void* _data;

  union {
    Ret (*_invoke_functor)(void*, Args...);
    Ret (*_invoke_ptr)(Args...);
  };
};

} // namespace shogle
