#pragma once

#include <shogle/core/common.hpp>

namespace ntf {

namespace impl {

template<ntf::not_void T,
  bool = std::is_trivially_destructible_v<T>>
class basic_uninitialized {
public:
  constexpr basic_uninitialized() noexcept :
    _dummy(), _inited(false) {}

  template<typename... Args>
  constexpr explicit basic_uninitialized(Args&&... args) 
  noexcept(std::is_nothrow_constructible_v<T, Args...>) :
    _obj(std::forward<Args>(args)...), _inited(true) {}

  template<typename U, typename... Args>
  constexpr explicit basic_uninitialized(std::initializer_list<U> l, Args&&... args)
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
class basic_uninitialized<T, false> {
public:
  constexpr basic_uninitialized() noexcept :
    _dummy(), _inited(false) {}

  template<typename... Args>
  constexpr explicit basic_uninitialized(Args&&... args) 
  noexcept(std::is_nothrow_constructible_v<T, Args...>) :
    _obj(std::forward<Args>(args)...), _inited(true) {}

  template<typename U, typename... Args>
  constexpr explicit basic_uninitialized(std::initializer_list<U> l, Args&&... args)
  noexcept(std::is_nothrow_constructible_v<std::initializer_list<U>, Args...>):
    _obj(l, std::forward<Args>(args)...), _inited(true) {}

public:
  constexpr ~basic_uninitialized()
  noexcept(std::is_nothrow_destructible_v<T>) {
    if (_inited) {
      _obj.~T();
    }
  }

public:
  constexpr basic_uninitialized(const basic_uninitialized&) = default;
  constexpr basic_uninitialized(basic_uninitialized&&) = default;
  basic_uninitialized& operator=(const basic_uninitialized&) = default;
  basic_uninitialized& operator=(basic_uninitialized&&) = default;

private:
  union {
    T _obj;
    char _dummy;
  };
  bool _inited;
};


template<typename T>
class uninitialized_ops : public basic_uninitialized<T> {
public:
  template<typename... Args>
  void init(Args&&... args)
  noexcept(std::is_nothrow_constructible_v<T, Args...>){
    NTF_ASSERT(!this->_inited, "uninitialized already initialized!");
    std::construct_at(std::addressof(this->_obj), std::forward<Args>(args)...);
    this->_inited = true;
  }

  void destroy()
  noexcept(std::is_nothrow_destructible_v<T>){
    NTF_ASSERT(this->_inited, "uninitialized not initialized!");
    std::destroy_at(std::addressof(this->_obj));
    this->_inited = false;
  }
  
public:
  bool inited() const noexcept { return this->_inited; }

  T& get() & noexcept {
    NTF_ASSERT(this->_inited, "uninitialized not initialized!");
    return *this;
  }
  const T& get() const& noexcept {
    NTF_ASSERT(this->_inited, "uninitialized not initialized!");
    return *this;
  }
  T&& get() && noexcept {
    NTF_ASSERT(this->_inited, "uninitialized not initialized!");
    return std::move(*this);
  }
  const T&& get() const&& noexcept {
    NTF_ASSERT(this->_inited, "uninitialized not initialized!");
    return std::move(*this);
  }
};


template<typename T,
  bool = std::is_trivially_copy_constructible_v<T>>
class uninitialized_copyc : public uninitialized_ops<T> {
public:
  using uninitialized_ops<T>::uninitialized_ops;
};

template<typename T>
class uninitialized_copyc<T, false> : public uninitialized_ops<T> {
public:
  using uninitialized_ops<T>::uninitialized_ops;

public:
  constexpr uninitialized_copyc(const uninitialized_copyc& u)
  noexcept(std::is_nothrow_copy_constructible_v<T>) {
    if (u.inited()) {
      this->init(u.get());
    }
  }

public:
  constexpr ~uninitialized_copyc() = default;
  constexpr uninitialized_copyc(uninitialized_copyc&&) = default;
  uninitialized_copyc& operator=(const uninitialized_copyc&) = default;
  uninitialized_copyc& operator=(uninitialized_copyc&&) = default;
};


template<typename T,
  bool = std::is_trivially_move_constructible_v<T>>
class uninitialized_movec : public uninitialized_copyc<T> {
public:
  using uninitialized_copyc<T>::uninitialized_copyc;
};

template<typename T>
class uninitialized_movec<T, false> : public uninitialized_copyc<T> {
public:
  using uninitialized_copyc<T>::uninitialized_copyc;

public:
  constexpr uninitialized_movec(uninitialized_movec&& u)
  noexcept(std::is_nothrow_move_constructible_v<T>) {
    if (u.inited()) {
      this->init(std::move(u.get()));
    }
  }

public:
  constexpr ~uninitialized_movec() = default;
  constexpr uninitialized_movec(const uninitialized_movec&) = default;
  uninitialized_movec& operator=(const uninitialized_movec&) = default;
  uninitialized_movec& operator=(uninitialized_movec&&) = default;
};


template<typename T,
  bool = std::is_trivially_copy_assignable_v<T>>
class uninitialized_copya : public uninitialized_movec<T> {
public:
  using uninitialized_movec<T>::uninitialized_movec;
};

template<typename T>
class uninitialized_copya<T, false> : public uninitialized_movec<T> {
public:
  using uninitialized_movec<T>::uninitialized_movec;

public:
  uninitialized_copya& operator=(const uninitialized_copya& u)
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
  constexpr ~uninitialized_copya() = default;
  constexpr uninitialized_copya(const uninitialized_copya&) = default;
  constexpr uninitialized_copya(uninitialized_copya&&) = default;
  uninitialized_copya& operator=(uninitialized_copya&&) = default;
};


template<typename T,
  bool = std::is_trivially_move_assignable_v<T>>
class uninitialized_movea : public uninitialized_copya<T> {
public:
  using uninitialized_copya<T>::uninitialized_copya;
};

template<typename T>
class uninitialized_movea<T, false> : public uninitialized_copya<T> {
public:
  using uninitialized_copya<T>::uninitialized_copya;

public:
  uninitialized_movea& operator=(uninitialized_movea&& u)
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
  constexpr ~uninitialized_movea() = default;
  constexpr uninitialized_movea(const uninitialized_movea&) = default;
  constexpr uninitialized_movea(uninitialized_movea&&) = default;
  uninitialized_movea& operator=(const uninitialized_movea&) = default;
};

} // namespace impl

template<typename T>
class uninitialized : public impl::uninitialized_movea<T> {
public:
  using impl::uninitialized_movea<T>::uninitialized_movea;

public:
  explicit operator bool() const noexcept { return this->inited(); }

  const T& operator*() const& noexcept { return this->get(); }
  T& operator*() & noexcept { return this->get(); }
  const T&& operator*() const&& noexcept { return std::move(this->get()); }
  T&& operator*() && noexcept { return std::move(this->get()); }

  const T* operator->() const noexcept { return std::addressof(this->get()); }
  T* operator->() noexcept { return std::addressof(this->get()); }
};

} // namespace ntf
