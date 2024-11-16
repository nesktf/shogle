#pragma once

#include <shogle/core/common.hpp>

#include <initializer_list>
#include <type_traits>
#include <utility>
#include <memory>

namespace ntf {

template<typename T, typename U>
concept is_forwarding = std::is_same_v<U, std::remove_cvref_t<T>>;

template<typename T>
concept not_void = !std::is_void_v<T>;

template<not_void E>
class unexpected {
public:
  unexpected() = delete;

  template<is_forwarding<E> U = E>
  constexpr unexpected(U&& err)
  noexcept(std::is_nothrow_constructible_v<E, U>) :
    _err(std::forward<U>(err)) {}

  template<typename... Args>
  requires(std::constructible_from<E, Args...>)
  constexpr explicit unexpected(Args&&... args)
  noexcept(std::is_nothrow_constructible_v<E, Args...>) :
    _err(std::forward<Args>(args)...) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<E, std::initializer_list<U>, Args...>)
  constexpr explicit unexpected(std::initializer_list<U> list, Args&&... args)
  noexcept(std::is_nothrow_constructible_v<E, std::initializer_list<U>, Args...>):
    _err(list, std::forward<Args>(args)...) {}

public:
  constexpr E& value() & noexcept { return _err; };
  constexpr const E& value() const& noexcept { return _err; }
  constexpr E&& value() && noexcept { return std::move(_err); };
  constexpr const E&& value() const&& noexcept { return std::move(_err); }

private:
  E _err;
};

template<typename E>
constexpr bool operator==(const unexpected<E>& lhs, const unexpected<E>& rhs) {
  return lhs.value() == rhs.value();
}

template<typename E>
constexpr bool operator!=(const unexpected<E>& lhs, const unexpected<E>& rhs) {
  return lhs.value() != rhs.value();
}

template<typename E>
constexpr bool operator<=(const unexpected<E>& lhs, const unexpected<E>& rhs) {
  return lhs.value() <= rhs.value();
}

template<typename E>
constexpr bool operator>=(const unexpected<E>& lhs, const unexpected<E>& rhs) {
  return lhs.value() >= rhs.value();
}

template<typename E>
constexpr bool operator<(const unexpected<E>& lhs, const unexpected<E>& rhs) {
  return lhs.value() < rhs.value();
}

template<typename E>
constexpr bool operator>(const unexpected<E>& lhs, const unexpected<E>& rhs) {
  return lhs.value() > rhs.value();
}

template<typename E>
unexpected(E) -> unexpected<E>;

template<typename E>
unexpected<typename std::decay_t<E>> make_unexpected(E&& e) {
  return unexpected<typename std::decay_t<E>>{std::forward<E>(e)};
}


struct in_place_t {
  constexpr explicit in_place_t() = default;
};
constexpr inline in_place_t in_place{};

struct unexpect_t {
  constexpr explicit unexpect_t() = default;
};
constexpr inline unexpect_t unexpect{};


namespace impl {

template<typename T, typename E,
  bool = std::is_trivially_destructible_v<T>,
  bool = std::is_trivially_destructible_v<E>>
class basic_expected_storage {
public:
  template<is_forwarding<T> U = T>
  requires(std::is_default_constructible_v<T>)
  constexpr basic_expected_storage()
  noexcept(std::is_nothrow_default_constructible_v<T>):
    _value(), _valid(true) {}

  template<typename... Args>
  requires(std::constructible_from<T, Args...>)
  constexpr explicit basic_expected_storage(in_place_t, Args&&... arg) 
  noexcept(std::is_nothrow_constructible_v<T, Args...>) :
    _value(std::forward<Args>(arg)...), _valid(true) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<T, std::initializer_list<U>, Args...>)
  constexpr explicit basic_expected_storage(in_place_t, std::initializer_list<U> l, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...>) :
    _value(l, std::forward<Args>(arg)...), _valid(true) {}

  template<typename... Args>
  requires(std::constructible_from<E, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, Args...>):
    _err(std::forward<Args>(arg)...), _valid(false) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<E, std::initializer_list<U>, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, std::initializer_list<U> l, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, std::initializer_list<U>, Args...>) :
    _err(l, std::forward<Args>(arg)...), _valid(false) {}

public:
  union {
    T _value;
    unexpected<E> _err;
  };
  bool _valid;
};

template<typename T, typename E>
class basic_expected_storage<T, E, true, false> {
public:
  template<is_forwarding<T> U = T>
  requires(std::is_default_constructible_v<T>)
  constexpr basic_expected_storage()
  noexcept(std::is_nothrow_default_constructible_v<T>):
    _value(), _valid(true) {}

  template<typename... Args>
  requires(std::constructible_from<T, Args...>)
  constexpr explicit basic_expected_storage(in_place_t, Args&&... arg) 
  noexcept(std::is_nothrow_constructible_v<T, Args...>) :
    _value(std::forward<Args>(arg)...), _valid(true) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<T, std::initializer_list<U>, Args...>)
  constexpr explicit basic_expected_storage(in_place_t, std::initializer_list<U> l, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...>) :
    _value(l, std::forward<Args>(arg)...), _valid(true) {}

  template<typename... Args>
  requires(std::constructible_from<E, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, Args...>):
    _err(std::forward<Args>(arg)...), _valid(false) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<E, std::initializer_list<U>, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, std::initializer_list<U> l, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, std::initializer_list<U>, Args...>) :
    _err(l, std::forward<Args>(arg)...), _valid(false) {}

public:
  constexpr ~basic_expected_storage()
  noexcept(std::is_nothrow_destructible_v<E>){
    if (!_valid) {
      _err.~unexpected<E>();
    }
  }

public:
  union {
    T _value;
    unexpected<E> _err;
  };
  bool _valid;
};

template<typename T, typename E>
class basic_expected_storage<T, E, false, true> {
public:
  template<is_forwarding<T> U = T>
  requires(std::is_default_constructible_v<T>)
  constexpr basic_expected_storage()
  noexcept(std::is_nothrow_default_constructible_v<T>):
    _value(), _valid(true) {}

  template<typename... Args>
  requires(std::constructible_from<T, Args...>)
  constexpr explicit basic_expected_storage(in_place_t, Args&&... arg) 
  noexcept(std::is_nothrow_constructible_v<T, Args...>) :
    _value(std::forward<Args>(arg)...), _valid(true) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<T, std::initializer_list<U>, Args...>)
  constexpr explicit basic_expected_storage(in_place_t, std::initializer_list<U> l, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...>) :
    _value(l, std::forward<Args>(arg)...), _valid(true) {}

  template<typename... Args>
  requires(std::constructible_from<E, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, Args...>):
    _err(std::forward<Args>(arg)...), _valid(false) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<E, std::initializer_list<U>, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, std::initializer_list<U> l, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, std::initializer_list<U>, Args...>) :
    _err(l, std::forward<Args>(arg)...), _valid(false) {}

public:
  constexpr ~basic_expected_storage()
  noexcept(std::is_nothrow_destructible_v<T>){
    if (_valid) {
      _value.~T();
    }
  }

public:
  union {
    T _value;
    unexpected<E> _err;
  };
  bool _valid;
};

template<typename T, typename E>
class basic_expected_storage<T, E, false, false> {
public:
  template<is_forwarding<T> U = T>
  requires(std::is_default_constructible_v<T>)
  constexpr basic_expected_storage()
  noexcept(std::is_nothrow_default_constructible_v<T>):
    _value(), _valid(true) {}

  template<typename... Args>
  requires(std::constructible_from<T, Args...>)
  constexpr explicit basic_expected_storage(in_place_t, Args&&... arg) 
  noexcept(std::is_nothrow_constructible_v<T, Args...>) :
    _value(std::forward<Args>(arg)...), _valid(true) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<T, std::initializer_list<U>, Args...>)
  constexpr explicit basic_expected_storage(in_place_t, std::initializer_list<U> l, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, Args...>) :
    _value(l, std::forward<Args>(arg)...), _valid(true) {}

  template<typename... Args>
  requires(std::constructible_from<E, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, Args...>):
    _err(std::forward<Args>(arg)...), _valid(false) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<E, std::initializer_list<U>, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, std::initializer_list<U> l, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, std::initializer_list<U>, Args...>) :
    _err(l, std::forward<Args>(arg)...), _valid(false) {}

public:
  constexpr ~basic_expected_storage()
  noexcept(std::is_nothrow_destructible_v<T> && std::is_nothrow_destructible_v<E>){
    if (_valid) {
      _value.~T();
    } else {
      _err.~unexpected<E>();
    }
  }

public:
  union {
    T _value;
    unexpected<E> _err;
  };
  bool _valid;
};

template<typename E>
class basic_expected_storage<void, E, false, true> {
public:
  constexpr basic_expected_storage()
  noexcept :
    _dummy(), _valid(true) {}

  template<typename... Args>
  constexpr explicit basic_expected_storage(in_place_t, Args&&...)
  noexcept :
    _dummy(), _valid(true) {}

  template<typename U, typename... Args>
  constexpr explicit basic_expected_storage(in_place_t, std::initializer_list<U>, Args&&...)
  noexcept :
    _dummy(), _valid(true) {}

  template<typename... Args>
  requires(std::constructible_from<E, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, Args...>):
    _err(std::forward<Args>(arg)...), _valid(false) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<E, std::initializer_list<U>, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, std::initializer_list<U> l, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, std::initializer_list<U>, Args...>) :
    _err(l, std::forward<Args>(arg)...), _valid(false) {}

public:
  union {
    char _dummy;
    unexpected<E> _err;
  };
  bool _valid;
};

template<typename E>
class basic_expected_storage<void, E, false, false> {
public:
  constexpr basic_expected_storage()
  noexcept :
    _dummy(), _valid(true) {}

  template<typename... Args>
  constexpr explicit basic_expected_storage(in_place_t, Args&&...)
  noexcept :
    _dummy(), _valid(true) {}

  template<typename U, typename... Args>
  constexpr explicit basic_expected_storage(in_place_t, std::initializer_list<U>, Args&&...)
  noexcept :
    _dummy(), _valid(true) {}

  template<typename... Args>
  requires(std::constructible_from<E, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, Args...>):
    _err(std::forward<Args>(arg)...), _valid(false) {}

  template<typename U, typename... Args>
  requires(std::constructible_from<E, std::initializer_list<U>, Args...>)
  constexpr explicit basic_expected_storage(unexpect_t, std::initializer_list<U> l, Args&&... arg)
  noexcept(std::is_nothrow_constructible_v<unexpected<E>, std::initializer_list<U>, Args...>) :
    _err(l, std::forward<Args>(arg)...), _valid(false) {}

public:
  constexpr ~basic_expected_storage()
  noexcept(std::is_nothrow_constructible_v<E>) {
    if (!_valid) {
      _err.~unexpected<E>();
    }
  }

public:
  union {
    char _dummy;
    unexpected<E> _err;
  };
  bool _valid;
};

template<typename T, typename E>
class expected_storage_ops : public basic_expected_storage<T, E> {
public:
  using basic_expected_storage<T, E>::basic_expected_storage;

public:
  template<typename... Args>
  constexpr void construct(Args&&... args) 
  noexcept(std::is_nothrow_constructible_v<T>) {
    std::construct_at(std::addressof(this->_value), std::forward<Args>(args)...);
    this->_valid = true;
  }

  template<typename... Args>
  constexpr void construct_error(Args&&... args)
  noexcept(std::is_nothrow_constructible_v<E>) {
    std::construct_at(std::addressof(this->_err), std::forward<Args>(args)...);
    this->_valid = false;
  }

public:
  constexpr bool has_value() const noexcept { return this->_valid; }

public:
  constexpr T& get() & noexcept { return this->_value; }
  constexpr const T& get() const& noexcept { return this->_value; }
  constexpr T&& get() && noexcept { return this->_value; }
  constexpr const T&& get() const&& noexcept { return this->_value; }

  constexpr unexpected<E>& get_error() & noexcept { return this->_err; }
  constexpr const unexpected<E>& get_error() const& noexcept { return this->_err; }
  constexpr unexpected<E>&& get_error() && noexcept { return this->_err; }
  constexpr const unexpected<E>&& get_error() const&& noexcept { return this->_err; }
};

template<typename E>
class expected_storage_ops<void, E> : public basic_expected_storage<void, E> {
public:
  using basic_expected_storage<void, E>::basic_expected_storage;

public:
  template<typename... Args>
  constexpr void construct(Args&&...) 
  noexcept {
    this->_valid = true;
  }

  template<typename... Args>
  constexpr void construct_error(Args&&... args)
  noexcept(std::is_nothrow_constructible_v<E>) {
    std::construct_at(std::addressof(this->_err), std::forward<Args>(args)...);
    this->_valid = false;
  }

public:
  constexpr bool has_value() const noexcept { return this->_valid; }

  constexpr unexpected<E>& get_error() & noexcept { return this->_err; }
  constexpr const unexpected<E>& get_error() const& noexcept { return this->_err; }
  constexpr unexpected<E>&& get_error() && noexcept { return this->_err; }
  constexpr const unexpected<E>&& get_error() const&& noexcept { return this->_err; }
};


template<typename T, typename E,
  bool = (std::is_trivially_copy_constructible_v<T> || std::is_same_v<T, void>)
  && std::is_trivially_copy_constructible_v<E>>
class expected_storage_copyc : public expected_storage_ops<T, E> {
public:
  using expected_storage_ops<T, E>::expected_storage_ops;
};

template<typename T, typename E>
class expected_storage_copyc<T, E, false> : public expected_storage_ops<T, E> {
public:
  using expected_storage_ops<T, E>::expected_storage_ops;

public:
  constexpr expected_storage_copyc(const expected_storage_copyc& e)
  noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_constructible_v<E>) {
    if (e.has_value()) {
      this->construct(e.get());
    } else {
      this->construct_error(e.get_error());
    }
  }

public:
  constexpr ~expected_storage_copyc() = default;
  constexpr expected_storage_copyc(expected_storage_copyc&&) = default;
  expected_storage_copyc& operator=(const expected_storage_copyc&) = default;
  expected_storage_copyc& operator=(expected_storage_copyc&&) = default;
};

template<typename T, typename E,
  bool = (std::is_trivially_move_constructible_v<T> || std::is_same_v<T, void>)
  && std::is_trivially_move_constructible_v<E>>
class expected_storage_movec : public expected_storage_copyc<T, E> {
public:
  using expected_storage_copyc<T, E>::expected_storage_copyc;
};

template<typename T, typename E>
class expected_storage_movec<T, E, false> : public expected_storage_copyc<T, E> {
public:
  using expected_storage_copyc<T, E>::expected_storage_copyc;

public:
  constexpr expected_storage_movec(expected_storage_movec&& e)
  noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E>) {
    if (e.has_value()) {
      this->construct(std::move(e.get()));
    } else {
      this->construct_error(std::move(e.get_error()));
    }
  }

public:
  constexpr ~expected_storage_movec() = default;
  constexpr expected_storage_movec(const expected_storage_movec&) = default;
  expected_storage_movec& operator=(const expected_storage_movec&) = default;
  expected_storage_movec& operator=(expected_storage_movec&) = default;
};


template<typename T, typename E,
  bool = (std::is_trivially_copy_assignable_v<T> || std::is_same_v<T, void>)
  && std::is_trivially_copy_assignable_v<E>>
class expected_storage_copya : public expected_storage_movec<T, E> {
public:
  using expected_storage_movec<T, E>::expected_storage_movec;
};

template<typename T, typename E>
class expected_storage_copya<T, E, false> : public expected_storage_movec<T, E> {
public:
  using expected_storage_movec<T, E>::expected_storage_movec;

public:
  expected_storage_copya& operator=(const expected_storage_copya& e)
  noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_assignable_v<E>
           && std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_constructible_v<E>) {
    // TODO: Manage leaks in case of exceptions?
    if (this->has_value()) {
      if (e.has_value()) {
        this->_value = e.get();
      } else {
        this->_value.~T();
        this->construct_error(e.get_error());
      }
    } else {
      if (e.has_value()) {
        this->_err.~unexpected<E>();
        this->construct(e.get());
      } else {
        this->_err = e.get_error();
      }
    }
    return *this;
  }

public:
  constexpr ~expected_storage_copya() = default;
  constexpr expected_storage_copya(const expected_storage_copya&) = default;
  constexpr expected_storage_copya(expected_storage_copya&&) = default;
  expected_storage_copya& operator=(expected_storage_copya&&) = default;
};

template<typename T, typename E,
  bool = (std::is_trivially_move_assignable_v<T> || std::is_same_v<T, void>)
  && std::is_trivially_move_assignable_v<T>>
class expected_storage_movea : public expected_storage_copya<T, E> {
public:
  using expected_storage_copya<T, E>::expected_storage_copya;
};

template<typename T, typename E>
class expected_storage_movea<T, E, false> : public expected_storage_copya<T, E> {
public:
  using expected_storage_copya<T, E>::expected_storage_copya;

public:
  expected_storage_movea& operator=(expected_storage_movea&& e)
  noexcept(std::is_trivially_move_assignable_v<T> && std::is_trivially_move_assignable_v<E>
      && std::is_trivially_move_constructible_v<T> && std::is_trivially_move_constructible_v<E>) {
    // TODO: Manage leaks in case of exceptions?
    if (this->has_value()) {
      if (e.has_value()) {
        this->_value = std::move(e.get());
      } else {
        this->_value.~T();
        this->construct_error(std::move(e.get_error()));
      }
    } else {
      if (e.has_value()) {
        this->_err.~unexpected<E>();
        this->construct(std::move(e.get()));
      } else {
        this->_err = std::move(e.get_error());
      }
    }
    return *this;
  }

public:
  constexpr ~expected_storage_movea() = default;
  constexpr expected_storage_movea(const expected_storage_movea&) = default;
  constexpr expected_storage_movea(expected_storage_movea&&) = default;
  expected_storage_movea& operator=(const expected_storage_movea&) = default;
};

template<typename T, typename E>
using expected_storage = expected_storage_movea<T, E>;

} // namespace impl

template<typename T, typename E>
class expected {
public:
  using value_type = T;
  using error_type = E;

  template<typename U>
  using rebind = expected<U, error_type>;

public:
  constexpr expected() = default;

  template<is_forwarding<T> U>
  constexpr expected(U&& val) :
    _storage(in_place, std::forward<U>(val)) {}

  template<is_forwarding<unexpected<E>> G>
  constexpr expected(G&& val) :
    _storage(unexpect, std::forward<G>(val).value()) {}

public:
  constexpr explicit operator bool() const noexcept { return _storage.has_value(); }
  constexpr bool has_value() const noexcept { return _storage.has_value(); }

  constexpr const T* operator->() const noexcept { return std::addressof(_storage.get()); }

  // TODO: Throw? when storage is invalid
  constexpr const T& operator*() const& { return _storage.get(); }
  constexpr T& operator*() & { return _storage.get(); }
  constexpr const T&& operator*() const&& { return std::move(_storage.get()); }
  constexpr T&& operator*() && { return std::move(_storage.get()); }

  constexpr const T& value() const& { return **this; }
  constexpr T& value() & { return **this; }
  constexpr const T&& value() const&& { return std::move(**this); }
  constexpr T&& value() && { return std::move(**this); }

  // TODO: Throw? when error is invalid
  constexpr const E& error() const& { return _storage.get_error().value(); }
  constexpr E& error() & { return _storage.get_error().value(); }
  constexpr const E&& error() const&& { return std::move(_storage.get_error().value()); }
  constexpr E&& error() && { return std::move(_storage.get_error().value()); }

  template<typename U>
  constexpr T value_or(U&& val) & {
    return _storage.has_value() ? **this : static_cast<T>(std::forward<U>(val));
  }

  template<typename U>
  constexpr T value_or(U&& val) && {
    return _storage.has_value() ?
      std::move(**this) : static_cast<T>(std::forward<U>(val));
  }

  template<typename U>
  constexpr E error_or(U&& err) & {
    return !_storage.has_value() ? 
      _storage.get_error().value() : static_cast<E>(std::forward<U>(err));
  }

  template<typename U>
  constexpr E error_or(U&& err) && {
    return !_storage.has_value() ?
      std::move(_storage.get_error().value()) : static_cast<E>(std::forward<U>(err));
  }

  // TODO: More monadic operations

private:
  impl::expected_storage<T, E> _storage;
};

} // namespace ntf
