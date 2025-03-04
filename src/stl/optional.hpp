#pragma once

#include "./types.hpp"

#if SHOGLE_USE_STL_OPTIONAL
#include <optional>
#endif

namespace ntf {

#if SHOGLE_USE_STL_OPTIONAL
template<typename T>
using optional = std::optional<T>;

using nullopt_t = std::nullopt_t;
constexpr nullopt_t nullopt = std::nullopt;

template<typename T>
struct is_optional : public std::false_type{};

template<typename T>
struct is_optional<optional<T>> : public std::true_type{};

template<typename T>
constexpr bool is_optional_v = is_optional<T>::value;

template<typename T>
concept is_optional_type = is_optional_v<T>;

#else
namespace impl {

template<ntf::not_void T,
  bool = std::is_trivially_destructible_v<T>>
class basic_optional {
public:
  constexpr basic_optional() noexcept :
    _dummy(), _inited(false) {}

  template<typename... Args>
  constexpr explicit basic_optional(Args&&... args) 
  noexcept(std::is_nothrow_constructible_v<T, Args...>) :
    _obj(std::forward<Args>(args)...), _inited(true) {}

  template<typename U, typename... Args>
  constexpr explicit basic_optional(std::initializer_list<U> l, Args&&... args)
  noexcept(std::is_nothrow_constructible_v<std::initializer_list<U>, Args...>):
    _obj(l, std::forward<Args>(args)...), _inited(true) {}

private:
  union {
    T _obj;
    char _dummy;
  };
  bool _inited;
};

template<ntf::not_void T>
class basic_optional<T, false> {
public:
  constexpr basic_optional() noexcept :
    _dummy(), _inited(false) {}

  template<typename... Args>
  constexpr explicit basic_optional(Args&&... args) 
  noexcept(std::is_nothrow_constructible_v<T, Args...>) :
    _obj(std::forward<Args>(args)...), _inited(true) {}

  template<typename U, typename... Args>
  constexpr explicit basic_optional(std::initializer_list<U> l, Args&&... args)
  noexcept(std::is_nothrow_constructible_v<std::initializer_list<U>, Args...>):
    _obj(l, std::forward<Args>(args)...), _inited(true) {}

public:
  constexpr ~basic_optional()
  noexcept(std::is_nothrow_destructible_v<T>) {
    if (_inited) {
      _obj.~T();
    }
  }

public:
  constexpr basic_optional(const basic_optional&) = default;
  constexpr basic_optional(basic_optional&&) = default;
  basic_optional& operator=(const basic_optional&) = default;
  basic_optional& operator=(basic_optional&&) = default;

private:
  union {
    T _obj;
    char _dummy;
  };
  bool _inited;
};


template<typename T>
class optional_ops : public basic_optional<T> {
public:
  template<typename... Args>
  void init(Args&&... args)
  noexcept(std::is_nothrow_constructible_v<T, Args...>){
    NTF_ASSERT(!this->_inited, "optional already initialized!");
    std::construct_at(std::addressof(this->_obj), std::forward<Args>(args)...);
    this->_inited = true;
  }

  void destroy()
  noexcept(std::is_nothrow_destructible_v<T>){
    NTF_ASSERT(this->_inited, "optional not initialized!");
    std::destroy_at(std::addressof(this->_obj));
    this->_inited = false;
  }
  
public:
  bool inited() const noexcept { return this->_inited; }

  T& get() & noexcept {
    NTF_ASSERT(this->_inited, "optional not initialized!");
    return *this;
  }
  const T& get() const& noexcept {
    NTF_ASSERT(this->_inited, "optional not initialized!");
    return *this;
  }
  T&& get() && noexcept {
    NTF_ASSERT(this->_inited, "optional not initialized!");
    return std::move(*this);
  }
  const T&& get() const&& noexcept {
    NTF_ASSERT(this->_inited, "optional not initialized!");
    return std::move(*this);
  }
};


template<typename T,
  bool = std::is_trivially_copy_constructible_v<T>>
class optional_copyc : public optional_ops<T> {
public:
  using optional_ops<T>::optional_ops;
};

template<typename T>
class optional_copyc<T, false> : public optional_ops<T> {
public:
  using optional_ops<T>::optional_ops;

public:
  constexpr optional_copyc(const optional_copyc& u)
  noexcept(std::is_nothrow_copy_constructible_v<T>) {
    if (u.inited()) {
      this->init(u.get());
    }
  }

public:
  constexpr ~optional_copyc() = default;
  constexpr optional_copyc(optional_copyc&&) = default;
  optional_copyc& operator=(const optional_copyc&) = default;
  optional_copyc& operator=(optional_copyc&&) = default;
};


template<typename T,
  bool = std::is_trivially_move_constructible_v<T>>
class optional_movec : public optional_copyc<T> {
public:
  using optional_copyc<T>::optional_copyc;
};

template<typename T>
class optional_movec<T, false> : public optional_copyc<T> {
public:
  using optional_copyc<T>::optional_copyc;

public:
  constexpr optional_movec(optional_movec&& u)
  noexcept(std::is_nothrow_move_constructible_v<T>) {
    if (u.inited()) {
      this->init(std::move(u.get()));
    }
  }

public:
  constexpr ~optional_movec() = default;
  constexpr optional_movec(const optional_movec&) = default;
  optional_movec& operator=(const optional_movec&) = default;
  optional_movec& operator=(optional_movec&&) = default;
};


template<typename T,
  bool = std::is_trivially_copy_assignable_v<T>>
class optional_copya : public optional_movec<T> {
public:
  using optional_movec<T>::optional_movec;
};

template<typename T>
class optional_copya<T, false> : public optional_movec<T> {
public:
  using optional_movec<T>::optional_movec;

public:
  optional_copya& operator=(const optional_copya& u)
  noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
    if (u.inited()) {
      if (this->inited()) {
        this->get() = u.get();
      } else {
        this->init(u.get());
      }
    } else if (this->inited()) {
      this->destroy();
    }
    return *this;
  }

public:
  constexpr ~optional_copya() = default;
  constexpr optional_copya(const optional_copya&) = default;
  constexpr optional_copya(optional_copya&&) = default;
  optional_copya& operator=(optional_copya&&) = default;
};


template<typename T,
  bool = std::is_trivially_move_assignable_v<T>>
class optional_movea : public optional_copya<T> {
public:
  using optional_copya<T>::optional_copya;
};

template<typename T>
class optional_movea<T, false> : public optional_copya<T> {
public:
  using optional_copya<T>::optional_copya;

public:
  optional_movea& operator=(optional_movea&& u)
  noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
    if (u.inited()) {
      if (this->inited()) {
        this->get() = std::move(u.get());
      } else {
        this->init(std::move(u.get()));
      }
    } else if (this->inited()) {
      this->destroy();
    }
    return *this;
  }

public:
  constexpr ~optional_movea() = default;
  constexpr optional_movea(const optional_movea&) = default;
  constexpr optional_movea(optional_movea&&) = default;
  optional_movea& operator=(const optional_movea&) = default;
};

} // namespace impl

template<typename T>
class optional : public impl::optional_movea<T> {
public:
  using impl::optional_movea<T>::optional_movea;

public:
  explicit operator bool() const noexcept { return this->inited(); }

  const T& operator*() const& noexcept { return this->get(); }
  T& operator*() & noexcept { return this->get(); }
  const T&& operator*() const&& noexcept { return std::move(this->get()); }
  T&& operator*() && noexcept { return std::move(this->get()); }

  const T* operator->() const noexcept { return std::addressof(this->get()); }
  T* operator->() noexcept { return std::addressof(this->get()); }
};
#endif

} // namespace ntf
