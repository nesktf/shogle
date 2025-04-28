#pragma once

#include "./types.hpp"
#include "../stl/allocator.hpp"

namespace ntf {

NTF_DECLARE_TAG_TYPE(uninitialized);

template<typename Deleter, typename T>
concept array_deleter_type = requires(Deleter& del, T* arr, size_t n) {
  noexcept(del(arr, n));
  { del(arr, n) } -> std::same_as<void>;
} || std::same_as<Deleter, std::default_delete<T[]>>;

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
class span {
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

public:
  constexpr span() noexcept :
    _data{nullptr}, _size{0u} {}

  constexpr explicit span(reference obj) noexcept :
    _data{std::addressof(obj)}, _size{1u} {}

  template<typename It>
  requires(std::contiguous_iterator<It>)
  constexpr span(It first, size_type count)
  requires(std::is_convertible_v<
    std::remove_reference_t<std::iter_reference_t<It>>(*)[],
   element_type(*)[]
  >) : _data{std::to_address(first)}, _size{count} {}

  template<typename It, typename End>
  requires(std::contiguous_iterator<It> && std::sized_sentinel_for<End, It>)
  constexpr span(It first, End last) 
  requires(std::is_convertible_v<
    std::remove_reference_t<std::iter_reference_t<It>>(*)[],
   element_type(*)[]
  >) : _data{std::to_address(first)}, _size{last-first} {}

  template<size_t N>
  constexpr span(std::type_identity_t<element_type>(&arr)[N]) noexcept
  requires(std::is_convertible_v<
    std::remove_pointer_t<decltype(std::data(arr))>(*)[],
    element_type(*)[]
  >) : _data{std::data(arr)}, _size{N} {}

  template<typename U, size_t N>
  constexpr span(std::array<U, N>& arr) noexcept
  requires(std::is_convertible_v<
    std::remove_pointer_t<decltype(std::data(arr))>(*)[],
    element_type(*)[]
  >) : _data{std::data(arr)}, _size{N} {}

  template<typename U, size_t N>
  constexpr span(const std::array<U, N>& arr) noexcept
  requires(std::is_convertible_v<
    std::remove_pointer_t<decltype(std::data(arr))>(*)[],
    element_type(*)[]
  >) : _data{std::data(arr)}, _size{N} {}

  constexpr span(std::initializer_list<value_type> il) noexcept :
    _data{il.begin()}, _size{il.size()} {}

  template<typename U>
  requires(std::is_convertible_v<U(*)[], element_type(*)[]>)
  constexpr span(const span<U>& src) noexcept :
    _data{src.data()}, _size{src.size()} {}

  constexpr span(const span&) noexcept = default;
  constexpr span(span&&) noexcept = default;

  constexpr ~span() noexcept = default;

public:
  constexpr span<element_type> first(size_type count) const {
    return {data(), count};
  }
  constexpr span<element_type> last(size_type count) const {
    return {data()+(size()-count), count};
  }

public:
  constexpr iterator begin() const noexcept { return _data; }
  constexpr const_iterator cbegin() const noexcept { return _data; }

  constexpr iterator end() const noexcept { return _data+_size; }
  constexpr const_iterator cend() const noexcept { return _data+_size; }

  constexpr reverse_iterator rbegin() const noexcept { return {_data+_size-1}; }
  constexpr const_reverse_iterator crbegin() const noexcept { return {_data+_size-1}; }

  constexpr reverse_iterator rend() const noexcept { return {_data-1}; }
  constexpr const_reverse_iterator crend() const noexcept { return {_data-1}; }

public:
  constexpr pointer data() const noexcept { return _data; }
  constexpr size_type size() const noexcept { return _size; }
  constexpr size_type size_bytes() const { return size()*sizeof(T); }

  constexpr reference front() const { return *begin(); }
  constexpr reference back() const { return *(end()-1); }

  constexpr bool empty() const noexcept { return size() == 0u; }
  constexpr pointer at(size_type idx) const noexcept {
    return idx >= size() ? nullptr : data()+idx;
  }

public:
  constexpr explicit operator bool() const noexcept { return !empty(); }
  constexpr reference operator[](size_type idx) const { return data()[idx]; }

  constexpr span& operator=(const span& other) noexcept = default;
  constexpr span& operator=(span&& other) noexcept = default;

  template<typename U>
  requires(std::is_convertible_v<U(*)[], element_type(*)[]>)
  constexpr span& operator=(const span<U>& other) noexcept {
    _data = other.data();
    _size = other.size();
    return *this;
  }

private:
  pointer _data;
  size_type _size;
};

template<typename T>
using span_view = span<const std::remove_cv_t<T>>;

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

namespace impl {

template<typename T, typename DelT>
struct unique_array_del : private DelT {
  unique_array_del() :
    DelT{} {}

  unique_array_del(const DelT& del) :
    DelT{del} {}

  unique_array_del(DelT&& del) :
    DelT{std::move(del)} {}

  void _delete_array(T* arr, size_t sz) noexcept {
    DelT::operator()(arr, sz);
  }

  DelT& _get_deleter() noexcept { return static_cast<DelT&>(*this); }
};

template<typename T> // Specialization for delete[]
struct unique_array_del<T, std::default_delete<T[]>> : private std::default_delete<T[]> {
  unique_array_del() noexcept {}

  unique_array_del(const std::default_delete<T[]>&) noexcept {}

  unique_array_del(std::default_delete<T[]>&&) noexcept {}

  void _delete_array(T* arr, size_t) noexcept {
    std::default_delete<T[]>::operator()(arr);
  }

  std::default_delete<T[]>& _get_deleter() noexcept {
    return static_cast<std::default_delete<T[]>&>(*this);
  }
};

} // namespace impl

template<typename T, array_deleter_type<T> DelT = default_alloc_del<T>>
requires(!std::is_pointer_v<T> && !std::is_reference_v<T>)
class unique_array : private impl::unique_array_del<T, DelT> {
public:
  using value_type = T;
  using deleter_type = DelT;
  using size_type = size_t;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = pointer;
  using const_iterator = const_pointer;

private:
  using del_base = impl::unique_array_del<T, DelT>;

public:
  constexpr unique_array()
  noexcept(std::is_nothrow_default_constructible_v<DelT>) :
    del_base{},
    _arr{nullptr}, _sz{0u} {}

  constexpr unique_array(std::nullptr_t)
  noexcept(std::is_nothrow_default_constructible_v<DelT>) :
    del_base{},
    _arr{nullptr}, _sz{0u} {}

  explicit unique_array(const DelT& del)
  noexcept(std::is_nothrow_copy_constructible_v<DelT>) :
    del_base{del},
    _arr{nullptr}, _sz{0u} {}

  explicit unique_array(DelT&& del)
  noexcept(std::is_nothrow_move_constructible_v<DelT>) :
    del_base{std::move(del)},
    _arr{nullptr}, _sz{0u} {}

  explicit unique_array(pointer arr, size_t sz)
  noexcept(std::is_nothrow_default_constructible_v<DelT>) :
    del_base{},
    _arr{arr}, _sz{sz} {}

  unique_array(pointer arr, size_t sz, const DelT& del)
  noexcept(std::is_nothrow_copy_constructible_v<DelT>) :
    del_base{del},
    _arr{arr}, _sz{sz} {}

  unique_array(pointer arr, size_t sz, DelT&& del)
  noexcept(std::is_nothrow_move_constructible_v<DelT>) :
    del_base{std::move(del)},
    _arr{arr}, _sz{sz} {}

  unique_array(unique_array&& other)
  noexcept(std::is_nothrow_move_constructible_v<DelT>) :
    del_base{static_cast<del_base&&>(other)},
    _arr{std::move(other._arr)}, _sz{std::move(other._sz)}
  {
    other._arr = nullptr;
    other._sz = 0u;
  }

  unique_array(const unique_array&) = delete;

  ~unique_array() noexcept { reset(); }

public:
  void reset(pointer arr, size_t sz) noexcept {
    if (_arr) {
      del_base::_delete_array(_arr, _sz);
    }
    _arr = arr;
    _sz = sz;
  }

  void reset() noexcept { reset(nullptr, 0u); }

  [[nodiscard]] std::pair<T*, size_t> release() noexcept {
    pointer ptr = get();
    size_t sz = size();
    _arr = nullptr;
    _sz = 0;
    return std::make_pair(ptr, sz);
  }

  template<typename F>
  void for_each(F&& fun) {
    if (!has_data()) {
      return;
    }
    for (auto it = begin(); it != end(); ++it) {
      fun(*it);
    }
  }

  template<typename F>
  void for_each(F&& fun) const {
    if (!has_data()) {
      return;
    }
    for (auto it = begin(); it != end(); ++it) {
      fun(*it);
    }
  }

public:
  unique_array& operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  unique_array& operator=(unique_array&& other)
    noexcept(std::is_nothrow_move_assignable_v<DelT>)
  {
    if (_arr) {
      del_base::_delete_array(_arr, _sz);
    }

    del_base::operator=(static_cast<del_base&&>(other));
    _arr = std::move(other._arr);
    _sz = std::move(other._sz);

    other._arr = nullptr;
    other._sz = 0u;

    return *this;
  }

  unique_array& operator=(const unique_array&) = delete;

  value_type& operator[](size_t idx) {
    NTF_ASSERT(idx < size());
    return get()[idx];
  }
  const value_type& operator[](size_t idx) const {
    NTF_ASSERT(idx < size());
    return get()[idx];
  }

  pointer at(size_t idx) noexcept {
    if (!has_data() || idx >= size()) {
      return nullptr;
    }
    return get()+idx;
  }

  const_pointer at(size_t idx) const noexcept {
    if (!has_data() || idx >= size()) {
      return nullptr;
    }
    return get()+idx;
  }

public:
  size_t size() const noexcept { return _sz; }

  pointer get() noexcept { return _arr; }
  const_pointer get() const noexcept { return _arr; }

  bool has_data() const noexcept { return get() != nullptr; }
  explicit operator bool() const noexcept { return has_data(); }

  DelT& get_deleter() noexcept { return del_base::_get_deleter(); }
  const DelT& get_deleter() const noexcept { del_base::_get_deleter(); }

  iterator begin() noexcept { return get(); }
  const_iterator begin() const noexcept { return get(); }
  const_iterator cbegin() const noexcept { return get(); }

  iterator end() noexcept { return get()+size(); }
  const_iterator end() const noexcept { return get()+size(); }
  const_iterator cend() const noexcept { return get()+size(); }

public:
  template<typename Cont>
  requires(std::copy_constructible<T>)
  static auto from_container(
    Cont&& container
  ) {
    using del_t = allocator_delete<T, std::remove_cvref_t<decltype(container.get_allocator())>>;

    pointer arr = nullptr;
    size_t sz = container.size();
    auto&& alloc = container.get_allocator();
    try {
      arr = alloc.allocate(sz);
      if (!arr) {
        return unique_array<T, del_t>{nullptr, 0u, del_t{alloc}};
      }
      size_t i = 0u;
      for (auto it = container.begin(); it != container.end(); ++i, ++it) {
        std::construct_at(arr+i, *it);
      }
    } catch (...) {
      alloc.deallocate(arr, sz);
      throw;
    }
    return unique_array<T, del_t>{arr, sz, del_t{alloc}};
  }

  template<typename Cont, allocator_type<T> Alloc>
  requires(allocator_type<std::remove_cvref_t<Alloc>, T> && std::copy_constructible<T>)
  static auto from_container(
    Cont&& container, Alloc&& alloc
  ) -> unique_array<T, allocator_delete<T, std::remove_cvref_t<Alloc>>> {
    using del_t = allocator_delete<T, std::remove_cvref_t<Alloc>>;

    pointer arr = nullptr;
    size_t sz = container.size();
    try {
      arr = alloc.allocate(sz);
      if (!arr) {
        return {nullptr, 0u, del_t{std::forward<Alloc>(alloc)}};
      }
      size_t i = 0u;
      for (auto it = container.begin(); it != container.end(); ++i, ++it) {
        std::construct_at(arr+i, *it);
      }
    } catch (...) {
      alloc.deallocate(arr, sz);
      throw;
    }
    return {arr, sz, del_t{alloc}};
  }

  template<typename Alloc = std::allocator<T>>
  requires(allocator_type<std::remove_cvref_t<Alloc>, T> && std::copy_constructible<T>)
  static auto from_allocator(
    size_t sz, const T& copy_obj, Alloc&& alloc = {}
  ) -> unique_array<T, allocator_delete<T, std::remove_cvref_t<Alloc>>>{
    using del_t = allocator_delete<T, std::remove_cvref_t<Alloc>>;

    pointer arr = nullptr;
    try {
      arr = alloc.allocate(sz);
      if (!arr) {
        return {nullptr, 0u, del_t{std::forward<Alloc>(alloc)}};
      }
      for (size_t i = 0; i < sz; ++i) {
        std::construct_at(arr+i, copy_obj);
      }
    } catch (...) {
      alloc.deallocate(arr, sz);
      throw;
    }
    return {arr, sz, del_t{std::forward<Alloc>(alloc)}};
  }

  template<typename Alloc = std::allocator<T>>
  requires(allocator_type<std::remove_cvref_t<Alloc>, T>)
  static auto from_allocator(
    uninitialized_t, size_t sz, Alloc&& alloc = {}
  ) -> unique_array<T, allocator_delete<T, std::remove_cvref_t<Alloc>>> {
    using del_t = allocator_delete<T, std::remove_cvref_t<Alloc>>;

    pointer arr = nullptr;
    try {
      arr = alloc.allocate(sz);
      if (!arr) {
        return {nullptr, 0u, del_t{std::forward<Alloc>(alloc)}};
      }
    } catch (...) {
      alloc.deallocate(arr, sz);
      throw;
    }
    return {arr, sz, del_t{std::forward<Alloc>(alloc)}};
  }

private:
  pointer _arr;
  size_t _sz;
};

NTF_DEFINE_TEMPLATE_CHECKER(unique_array);

template<typename T>
class virtual_array_deleter {
public:
  static constexpr size_t BUFFER_SIZE = 2*sizeof(void*);

private:
  struct vtable_t {
    void (*deleter)(void*, T*, size_t);
    void (*destroy)(void*);
    void (*copy)(void*, const void*);
  };

  template<typename U>
  static constexpr vtable_t vtable_for {
    .deleter = +[](void* data, T* ptr, size_t count) {
      (*static_cast<U*>(data))(ptr, count);
    },
    .destroy = +[](void* data) {
      static_assert(std::is_destructible_v<U>, "Deleter has to be destructible");
      static_assert(std::is_nothrow_destructible_v<U>, "Deleter has to be nothrow destructible");
      static_cast<U*>(data)->~U();
    },
    .copy = +[](void* data, const void* other) {
      static_assert(std::copy_constructible<U>, "Deleter has to be copy constructible");
      std::construct_at(static_cast<U*>(data), *static_cast<const U*>(other));
    },
  };

public:
  template<typename U>
  requires(array_deleter_type<std::remove_cvref_t<U>, T>
           && sizeof(std::remove_cvref_t<U>) <= BUFFER_SIZE)
  virtual_array_deleter(U&& del)
  noexcept(is_nothrow_forward_constructible<U>) :
    _vtable{&vtable_for<std::remove_cvref_t<U>>}
  {
    _construct<std::remove_cvref_t<U>>(std::forward<U>(del));
  }

  template<typename U, typename... Args>
  requires(array_deleter_type<std::remove_cvref_t<U>, T>
           && sizeof(std::remove_cvref_t<U>) <= BUFFER_SIZE)
  virtual_array_deleter(std::in_place_type_t<U>, Args&&... args)
  noexcept(std::is_nothrow_constructible_v<U, Args...>) :
    _vtable{&vtable_for<std::remove_cvref_t<U>>}
  {
    _construct<std::remove_cvref_t<U>>(std::forward<Args>(args)...);
  }

  template<typename U, typename L, typename... Args>
  requires(array_deleter_type<std::remove_cvref_t<U>, T>
           && sizeof(std::remove_cvref_t<U>) <= BUFFER_SIZE)
  virtual_array_deleter(std::in_place_type_t<U>, std::initializer_list<L> il, Args&&... args)
  noexcept(std::is_nothrow_constructible_v<U, std::initializer_list<L>, Args...>) :
    _vtable{&vtable_for<std::remove_cvref_t<U>>}
  {
    _construct<std::remove_cvref_t<U>>(il, std::forward<Args>(args)...);
  }

  virtual_array_deleter(const virtual_array_deleter& other) :
    _vtable{other._vtable}
  {
    _copy(&other._buffer[0]);
  }

  virtual_array_deleter(virtual_array_deleter&& other) noexcept :
    _vtable{std::move(other._vtable)}
  {
    _move(&other._buffer[0]);
    other._vtable = nullptr;
  }

  ~virtual_array_deleter() noexcept { _destroy(); }

public:
  void operator()(T* ptr, size_t count) noexcept {
    if (_vtable) {
      std::invoke(_vtable->deleter, reinterpret_cast<void*>(&_buffer[0]), ptr, count);
    }
  }

public:
  virtual_array_deleter& operator=(const virtual_array_deleter& other) {
    _destroy();

    _vtable = other._vtable;
    _copy(&other._buffer[0]);

    return *this;
  }

  virtual_array_deleter& operator=(virtual_array_deleter&& other) noexcept {
    _destroy();
    
    _vtable = std::move(other._vtable);
    _move(&other._buffer[0]);

    other._vtable = nullptr;

    return *this;
  }

private:
  template<typename U, typename... Args>
  void _construct(Args&&... args) {
    std::construct_at(reinterpret_cast<U*>(&_buffer[0]), std::forward<Args>(args)...);
  }

  void _copy(const uint8* other_buf) {
    std::invoke(_vtable->copy,
                reinterpret_cast<void*>(&_buffer[0]), reinterpret_cast<const void*>(other_buf));
  }

  void _move(uint8* other_buf) {
    std::memcpy(&_buffer[0], other_buf, BUFFER_SIZE);
  }

  void _destroy() noexcept {
    if (_vtable) {
      std::invoke(_vtable->destroy, reinterpret_cast<void*>(&_buffer[0]));
    }
  }

private:
  uint8 _buffer[BUFFER_SIZE];
  const vtable_t* _vtable;
};

template<typename T>
using virtual_unique_array = unique_array<T, virtual_alloc_del<T>>;

template<typename Arr, typename T>
struct unique_array_check_t : public std::false_type {};
template<typename T, typename DelT>
struct unique_array_check_t<unique_array<T, DelT>, T> : public std::true_type {};
template<typename Arr, typename T>
constexpr bool unique_array_check_t_v = unique_array_check_t<Arr, T>::value;
template<typename Arr, typename T>
concept unique_array_with_type = unique_array_check_t_v<Arr, T>;

} // namespace ntf
