#pragma once

#include "./types.hpp"

#if !defined(NDEBUG)
#define NTF_EXPECTED_THROW(cond, ...) \
  NTF_ASSERT(cond, "bad_unexpected_access")
#define NTF_EXPECTED_NOEXCEPT noexcept
#else
#if defined(NTF_EXPECTED_ENABLE_EXCEPTIONS) && NTF_EXPECTED_ENABLE_EXCEPTIONS
#define NTF_EXPECTED_THROW(cond, T, ...) \
  if (!cond) { throw ::ntf::bad_expected_access<T>{__VA_ARGS__}; }
#define NTF_EXPECTED_NOEXCEPT
#else
#define NTF_EXPECTED_THROW(cond, ...)
#define NTF_EXPECTED_NOEXCEPT noexcept
#endif
#endif

namespace ntf {

template<typename>
class bad_expected_access;

template<>
class bad_expected_access<void> : public std::exception {
public:
  bad_expected_access() = default;

public:
  const char* what() const noexcept override { return "bad_expected_access"; }
};

template<typename E>
class bad_expected_access : public bad_expected_access<void> {
public:
  explicit bad_expected_access(E err) :
    _err{std::move(err)} {}

public:
  E& error() & noexcept { return _err; }
  const E& error() const& noexcept { return _err; }
  E&& error() && noexcept { return std::move(_err); }
  const E&& error() const&& noexcept { return std::move(_err); }

private:
  E _err;
};

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

NTF_DECLARE_TAG_TYPE(unexpect);

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

template<typename, typename>
struct expected_catcher {};

template<typename T, typename Expected>
struct expected_catcher<::ntf::error<T>, Expected> {
  template<is_forwarding<T> U, typename Fun>
  static Expected catch_error(U&& obj, Fun&& f,
                              std::source_location l = std::source_location::current()) noexcept {
    try {
      return f();
    } catch (::ntf::error<T>& ex) {
      return unexpected{std::move(ex)};
    } catch (const std::exception& ex) {
      return unexpected{::ntf::error<T>{std::forward<T>(obj), ex.what(), l}};
    } catch (...) {
      return unexpected{::ntf::error<T>{std::forward<T>(obj), "Unknown error", l}};
    }
  }
};

template<typename Expected>
struct expected_catcher<::ntf::error<void>, Expected> {
  template<typename Fun>
  static Expected catch_error(Fun&& f,
                              std::source_location l = std::source_location::current()) noexcept {
    try {
      return f();
    } catch (::ntf::error<void>& ex) {
      return unexpected{std::move(ex)};
    } catch (const std::exception& ex) {
      return unexpected{::ntf::error<void>{ex.what(), l}};
    } catch (...) {
      return unexpected{::ntf::error<void>{"Unknown error", l}};
    }
  }
};

} // namespace impl

template<typename, typename>
class expected;

template<typename T>
struct is_expected : public std::false_type {};

template<typename T>
struct is_expected<expected<T, void>> : public std::false_type {};

template<typename T, typename E>
struct is_expected<expected<T, E>> : public std::true_type {};

template<typename T>
constexpr bool is_expected_v = is_expected<T>::value;


template<typename Exp, typename T>
struct is_expected_t : public std::false_type{};

template<typename T>
struct is_expected_t<expected<T, void>, T> : public std::false_type{};

template<typename T, typename E>
struct is_expected_t<expected<T, E>, T> : public std::true_type{};

template<typename Exp, typename T>
constexpr bool is_expected_t_v = is_expected_t<Exp, T>::value;


template<typename Exp, typename E>
struct is_expected_e : public std::false_type{};

template<typename T>
struct is_expected_e<expected<T, void>, void> : public std::false_type{};

template<typename T, typename E>
struct is_expected_e<expected<T, E>, E> : public std::true_type{};

template<typename Exp, typename E>
constexpr bool is_expected_e_v = is_expected_e<Exp, E>::value;


template<typename T>
concept is_expected_type = is_expected_v<T>;

template<typename T, typename Exp>
concept is_expected_with_type = is_expected_t_v<Exp, T>;

template<typename E, typename Exp>
concept is_expected_with_error = is_expected_e_v<Exp, E>;


template<typename T, typename E>
class expected : public impl::expected_catcher<E, expected<T, E>> {
public:
  using value_type = T;
  using error_type = E;

  template<typename U>
  using rebind = expected<U, error_type>;

public:
  constexpr expected() = default;

  template<typename... Args>
  constexpr expected(in_place_t, Args&&... args) :
    _storage(in_place, std::forward<Args>(args)...) {}

  template<typename U, typename... Args>
  constexpr expected(in_place_t, std::initializer_list<U> l, Args&&... args) :
    _storage(in_place, l, std::forward<Args>(args)...) {}

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
  constexpr T* operator->() noexcept { return std::addressof(_storage.get()); }

  constexpr const T& operator*() const& NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(*this, E, std::as_const(_storage.get_error().value()));
    return _storage.get();
  }
  constexpr T& operator*() & NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(*this, E, std::as_const(_storage.get_error().value()));
    return _storage.get();
  }
  constexpr const T&& operator*() const&& NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(*this, E, std::move(_storage.get_error().value()));
    return std::move(_storage.get());
  }
  constexpr T&& operator*() && NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(*this, E, std::move(_storage.get_error().value()));
    return std::move(_storage.get());
  }

  constexpr const T& value() const& { return **this; }
  constexpr T& value() & { return **this; }
  constexpr const T&& value() const&& { return std::move(**this); }
  constexpr T&& value() && { return std::move(**this); }

  constexpr const E& error() const& NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(!*this, void);
    return _storage.get_error().value();
  }
  constexpr E& error() & NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(!*this, void);
    return _storage.get_error().value();
  }
  constexpr const E&& error() const&& NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(!*this, void);
    return std::move(_storage.get_error().value());
  }
  constexpr E&& error() && NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(!*this, void);
    return std::move(_storage.get_error().value());
  }

public:
  template<typename... Args>
  requires(std::is_nothrow_constructible_v<T, Args...>)
  constexpr T& emplace(Args&&... args) noexcept {
    _storage.get().~T();
    _storage.construct(std::forward<Args>(args)...);
    return *this;
  }

  template<typename U, typename... Args>
  requires(std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>)
  constexpr T& emplace(std::initializer_list<U> l, Args&&... args) noexcept {
    _storage.get().~T();
    _storage.construct(l, std::forward<Args>(args)...);
    return *this;
  }

  template<typename U = std::remove_cv_t<T>>
  requires(std::convertible_to<T, U> && std::is_copy_constructible_v<T>)
  constexpr T value_or(U&& val) const& {
    return *this ?
      _storage.get() : static_cast<T>(std::forward<U>(val));
  }

  template<typename U = std::remove_cv_t<T>>
  requires(std::convertible_to<T, U> && std::is_move_constructible_v<T>)
  constexpr T value_or(U&& val) && {
    return *this ?
      std::move(_storage.get()) : static_cast<T>(std::forward<U>(val));
  }

  template<typename G = E>
  requires(std::convertible_to<E, G> && std::is_copy_constructible_v<E>)
  constexpr E error_or(G&& err) const& {
    return !*this ?
      _storage.get_error().value() : static_cast<E>(std::forward<G>(err));
  }

  template<typename G = E>
  requires(std::convertible_to<E, G> && std::is_move_constructible_v<E>)
  constexpr E error_or(G&& err) && {
    return !*this ?
      std::move(_storage.get_error().value()) : static_cast<E>(std::forward<G>(err));
  }

public:
  template<typename F>
  requires(std::is_invocable_v<F, T>)
  constexpr auto and_then(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F, decltype(_storage.get())>>;
    static_assert(is_expected_with_error<E, U>, "F needs to return an expected with error E");

    if (*this) {
      return std::invoke(std::forward<F>(f), _storage.get());
    } else {
      return U{unexpect, _storage.get_error().value()};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, T>)
  constexpr auto and_then(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, decltype(_storage.get())>>;
    static_assert(is_expected_with_error<E, U>, "F needs to return an expected with error E");

    if (*this) {
      return std::invoke(std::forward<F>(f), _storage.get());
    } else {
      return U{unexpect, _storage.get_error().value()};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, T>)
  constexpr auto and_then(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F, decltype(std::move(_storage.get()))>>;
    static_assert(is_expected_with_error<E, U>, "F needs to return an expected with error E");

    if (*this) {
      return std::invoke(std::forward<F>(f), std::move(_storage.get()));
    } else {
      return U{unexpect, std::move(_storage.get_error().value())};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, T>)
  constexpr auto and_then(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F, decltype(std::move(_storage.get()))>>;
    static_assert(is_expected_with_error<E, U>, "F needs to return an expected with error E");

    if (*this) {
      return std::invoke(std::forward<F>(f), std::move(_storage.get()));
    } else {
      return U{unexpect, std::move(_storage.get_error().value())};
    }
  }


  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto or_else(F&& f) & {
    using G = std::remove_cvref_t<std::invoke_result_t<F, decltype(error())>>;
    static_assert(is_expected_with_error<E, G>, "F needs to return an expected with error E");

    if (*this) {
      return G{in_place, _storage.get()};
    } else {
      return std::invoke(std::forward<F>(f), _storage.get_error().value());
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto or_else(F&& f) const& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, decltype(error())>>;
    static_assert(is_expected_with_error<E, G>, "F needs to return an expected with error E");

    if (*this) {
      return G{in_place, _storage.get()};
    } else {
      return std::invoke(std::forward<F>(f), _storage.get_error().value());
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto or_else(F&& f) && {
    using G = std::remove_cvref_t<std::invoke_result_t<F, decltype(error())>>;
    static_assert(is_expected_with_error<E, G>, "F needs to return an expected with error E");

    if (*this) {
      return G{in_place, std::move(_storage.get())};
    } else {
      return std::invoke(std::forward<F>(f), std::move(_storage.get_error().value()));
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto or_else(F&& f) const&& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, decltype(error())>>;
    static_assert(is_expected_with_error<E, G>, "F needs to return an expected with error E");

    if (*this) {
      return G{in_place, std::move(_storage.get())};
    } else {
      return std::invoke(std::forward<F>(f), std::move(_storage.get_error().value()));
    }
  }


  template<typename F>
  requires(std::is_invocable_v<F, T>)
  constexpr auto transform(F&& f) & {
    using U = std::remove_cv_t<std::invoke_result_t<F, decltype(_storage.get())>>;
    static_assert(!std::is_lvalue_reference_v<U> && std::is_rvalue_reference_v<U>,
                  "F can't return a reference type");

    if (*this) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f), _storage.get());
        return expected<void, E>{};
      } else {
        return expected<U, E>{in_place, std::invoke(std::forward<F>(f), _storage.get())};
      }
    } else {
      return expected<U, E>{unexpect, _storage.get_error().value()};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, T>)
  constexpr auto transform(F&& f) const& {
    using U = std::remove_cv_t<std::invoke_result_t<F, decltype(_storage.get())>>;
    static_assert(!std::is_lvalue_reference_v<U> && std::is_rvalue_reference_v<U>,
                  "F can't return a reference type");

    if (*this) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f), _storage.get());
        return expected<void, E>{};
      } else {
        return expected<U, E>{in_place, std::invoke(std::forward<F>(f), _storage.get())};
      }
    } else {
      return expected<U, E>{unexpect, _storage.get_error().value()};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, T>)
  constexpr auto transform(F&& f) && {
    using U = std::remove_cv_t<std::invoke_result_t<F, decltype(std::move(_storage.get()))>>;
    static_assert(!std::is_lvalue_reference_v<U> && std::is_rvalue_reference_v<U>,
                  "F can't return a reference type");

    if (*this) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f), std::move(_storage.get()));
        return expected<void, E>{};
      } else {
        return expected<U, E>{in_place,
                              std::invoke(std::forward<F>(f), std::move(_storage.get()))};
      }
    } else {
      return expected<U, E>{unexpect, std::move(_storage.get_error().value())};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, T>)
  constexpr auto transform(F&& f) const&& {
    using U = std::remove_cv_t<std::invoke_result_t<F, decltype(std::move(_storage.get()))>>;
    static_assert(!std::is_lvalue_reference_v<U> && std::is_rvalue_reference_v<U>,
                  "F can't return a reference type");

    if (*this) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f), std::move(_storage.get()));
        return expected<void, E>{};
      } else {
        return expected<U, E>{in_place,
                              std::invoke(std::forward<F>(f), std::move(_storage.get()))};
      }
    } else {
      return expected<U, E>{unexpect, std::move(_storage.get_error().value())};
    }
  }


  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto transform_error(F&& f) & {
    using G = std::remove_cv_t<std::invoke_result_t<F, decltype(error())>>;
    static_assert(!std::is_void_v<G>, "F has to return an error value");
    static_assert(!std::is_lvalue_reference_v<G> && std::is_rvalue_reference_v<G>,
                  "F can't return a reference type");

    if (*this) {
      return expected<T, G>{in_place, _storage.get()};
    } else {
      return expected<T, G>{
        unexpect,
        std::invoke(std::forward<F>(f), _storage.get_error().value())
      };
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto transform_error(F&& f) const& {
    using G = std::remove_cv_t<std::invoke_result_t<F, decltype(error())>>;
    static_assert(!std::is_void_v<G>, "F has to return an error value");
    static_assert(!std::is_lvalue_reference_v<G> && std::is_rvalue_reference_v<G>,
                  "F can't return a reference type");

    if (*this) {
      return expected<T, G>{in_place, _storage.get()};
    } else {
      return expected<T, G>{
        unexpect,
        std::invoke(std::forward<F>(f), _storage.get_error().value())
      };
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto transform_error(F&& f) && {
    using G = std::remove_cv_t<std::invoke_result_t<F, decltype(std::move(error()))>>;
    static_assert(!std::is_void_v<G>, "F has to return an error value");
    static_assert(!std::is_lvalue_reference_v<G> && std::is_rvalue_reference_v<G>,
                  "F can't return a reference type");

    if (*this) {
      return expected<T, G>{in_place, std::move(_storage.get())};
    } else {
      return expected<T, G>{
        unexpect,
        std::invoke(std::forward<F>(f), std::move(_storage.get_error().value()))
      };
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto transform_error(F&& f) const&& {
    using G = std::remove_cv_t<std::invoke_result_t<F, decltype(std::move(error()))>>;
    static_assert(!std::is_void_v<G>, "F has to return an error value");
    static_assert(!std::is_lvalue_reference_v<G> && std::is_rvalue_reference_v<G>,
                  "F can't return a reference type");

    if (*this) {
      return expected<T, G>{in_place, std::move(_storage.get())};
    } else {
      return expected<T, G>{
        unexpect,
        std::invoke(std::forward<F>(f), std::move(_storage.get_error().value()))
      };
    }
  }

private:
  impl::expected_storage<T, E> _storage;
};

template<typename E>
class expected<void, E> : public impl::expected_catcher<E, expected<void, E>> {
public:
  using value_type = void;
  using error_type = E;

  template<typename U>
  using rebind = expected<U, error_type>;

public:
  constexpr expected() = default;

  constexpr expected(in_place_t) :
    _storage(in_place) {}

  template<typename... Args>
  constexpr expected(unexpect_t, Args&&... args) :
    _storage(unexpect, std::forward<Args>(args)...) {}

  template<typename U, typename... Args>
  constexpr expected(unexpect_t, std::initializer_list<U> l, Args&&... args) :
    _storage(unexpect, l, std::forward<Args>(args)...) {}

  template<is_forwarding<unexpected<E>> G>
  constexpr expected(G&& val) :
    _storage(unexpect, std::forward<G>(val).value()) {}

public:
  constexpr explicit operator bool() const noexcept { return _storage.has_value(); }
  constexpr bool has_value() const noexcept { return _storage.has_value(); }

  constexpr const E& error() const& NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(!*this, void);
    return _storage.get_error().value();
  }
  constexpr E& error() & NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(!*this, void);
    return _storage.get_error().value();
  }
  constexpr const E&& error() const&& NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(!*this, void);
    return std::move(_storage.get_error().value());
  }
  constexpr E&& error() && NTF_EXPECTED_NOEXCEPT
  {
    NTF_EXPECTED_THROW(!*this, void);
    return std::move(_storage.get_error().value());
  }

public:
  constexpr void emplace() noexcept { _storage = {in_place}; }

  template<typename G = E>
  requires(std::convertible_to<E, G> && std::is_copy_constructible_v<E>)
  constexpr E error_or(G&& err) const& {
    return !*this ?
      _storage.get_error().value() : static_cast<E>(std::forward<G>(err));
  }

  template<typename G = E>
  requires(std::convertible_to<E, G> && std::is_move_constructible_v<E>)
  constexpr E error_or(G&& err) && {
    return !*this ?
      std::move(_storage.get_error().value()) : static_cast<E>(std::forward<G>(err));
  }

public:
  template<typename F>
  requires(std::is_invocable_v<F>)
  constexpr auto and_then(F&& f) & {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_expected_with_error<E, U>, "F needs to return an expected with error E");

    if (*this) {
      return std::invoke(std::forward<F>(f));
    } else {
      return U{unexpect, _storage.get_error().value()};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F>)
  constexpr auto and_then(F&& f) const& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_expected_with_error<E, U>, "F needs to return an expected with error E");

    if (*this) {
      return std::invoke(std::forward<F>(f));
    } else {
      return U{unexpect, _storage.get_error().value()};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F>)
  constexpr auto and_then(F&& f) && {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_expected_with_error<E, U>, "F needs to return an expected with error E");

    if (*this) {
      return std::invoke(std::forward<F>(f));
    } else {
      return U{unexpect, std::move(_storage.get_error().value())};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F>)
  constexpr auto and_then(F&& f) const&& {
    using U = std::remove_cvref_t<std::invoke_result_t<F>>;
    static_assert(is_expected_with_error<E, U>, "F needs to return an expected with error E");

    if (*this) {
      return std::invoke(std::forward<F>(f));
    } else {
      return U{unexpect, std::move(_storage.get_error().value())};
    }
  }


  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto or_else(F&& f) & {
    using G = std::remove_cvref_t<std::invoke_result_t<F, decltype(error())>>;
    static_assert(is_expected_with_error<E, G>, "F needs to return an expected with error E");

    if (*this) {
      return G{};
    } else {
      return std::invoke(std::forward<F>(f), _storage.get_error().value());
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto or_else(F&& f) const& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, decltype(error())>>;
    static_assert(is_expected_with_error<E, G>, "F needs to return an expected with error E");

    if (*this) {
      return G{};
    } else {
      return std::invoke(std::forward<F>(f), _storage.get_error().value());
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto or_else(F&& f) && {
    using G = std::remove_cvref_t<std::invoke_result_t<F, decltype(std::move(error()))>>;
    static_assert(is_expected_with_error<E, G>, "F needs to return an expected with error E");

    if (*this) {
      return G{};
    } else {
      return std::invoke(std::forward<F>(f), std::move(_storage.get_error().value()));
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto or_else(F&& f) const&& {
    using G = std::remove_cvref_t<std::invoke_result_t<F, decltype(std::move(error()))>>;
    static_assert(is_expected_with_error<E, G>, "F needs to return an expected with error E");

    if (*this) {
      return G{};
    } else {
      return std::invoke(std::forward<F>(f), std::move(_storage.get_error().value()));
    }
  }


  template<typename F>
  requires(std::is_invocable_v<F>)
  constexpr auto transform(F&& f) & {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    static_assert(!std::is_lvalue_reference_v<U> && std::is_rvalue_reference_v<U>,
                  "F can't return a reference type");

    if (*this) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f));
        return expected<void, E>{};
      } else {
        return expected<U, E>{in_place, std::invoke(std::forward<F>(f))};
      }
    } else {
      return expected<U, E>{unexpect, _storage.get_error().value()};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F>)
  constexpr auto transform(F&& f) const& {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    static_assert(!std::is_lvalue_reference_v<U> && std::is_rvalue_reference_v<U>,
                  "F can't return a reference type");

    if (*this) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f));
        return expected<void, E>{};
      } else {
        return expected<U, E>{in_place, std::invoke(std::forward<F>(f))};
      }
    } else {
      return expected<U, E>{unexpect, _storage.get_error().value()};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F>)
  constexpr auto transform(F&& f) && {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    static_assert(!std::is_lvalue_reference_v<U> && std::is_rvalue_reference_v<U>,
                  "F can't return a reference type");

    if (*this) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f));
        return expected<void, E>{};
      } else {
        return expected<U, E>{in_place, std::invoke(std::forward<F>(f))};
      }
    } else {
      return expected<U, E>{unexpect, std::move(_storage.get_error().value())};
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F>)
  constexpr auto transform(F&& f) const&& {
    using U = std::remove_cv_t<std::invoke_result_t<F>>;
    static_assert(!std::is_lvalue_reference_v<U> && std::is_rvalue_reference_v<U>,
                  "F can't return a reference type");

    if (*this) {
      if constexpr (std::is_void_v<U>) {
        std::invoke(std::forward<F>(f));
        return expected<void, E>{};
      } else {
        return expected<U, E>{in_place, std::invoke(std::forward<F>(f))};
      }
    } else {
      return expected<U, E>{unexpect, std::move(_storage.get_error().value())};
    }
  }


  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto transform_error(F&& f) & {
    using G = std::remove_cv_t<std::invoke_result_t<F, decltype(error())>>;
    static_assert(!std::is_void_v<G>, "F has to return an error value");
    static_assert(!std::is_lvalue_reference_v<G> && std::is_rvalue_reference_v<G>,
                  "F can't return a reference type");

    if (*this) {
      return expected<void, G>{};
    } else {
      return expected<void, G>{
        unexpect,
        std::invoke(std::forward<F>(f), _storage.get_error().value())
      };
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto transform_error(F&& f) const& {
    using G = std::remove_cv_t<std::invoke_result_t<F, decltype(error())>>;
    static_assert(!std::is_void_v<G>, "F has to return an error value");
    static_assert(!std::is_lvalue_reference_v<G> && std::is_rvalue_reference_v<G>,
                  "F can't return a reference type");

    if (*this) {
      return expected<void, G>{};
    } else {
      return expected<void, G>{
        unexpect,
        std::invoke(std::forward<F>(f), _storage.get_error().value())
      };
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto transform_error(F&& f) && {
    using G = std::remove_cv_t<std::invoke_result_t<F, decltype(std::move(error()))>>;
    static_assert(!std::is_void_v<G>, "F has to return an error value");
    static_assert(!std::is_lvalue_reference_v<G> && std::is_rvalue_reference_v<G>,
                  "F can't return a reference type");

    if (*this) {
      return expected<void, G>{};
    } else {
      return expected<void, G>{
        unexpect,
        std::invoke(std::forward<F>(f), std::move(_storage.get_error().value()))
      };
    }
  }
  template<typename F>
  requires(std::is_invocable_v<F, E>)
  constexpr auto transform_error(F&& f) const&& {
    using G = std::remove_cv_t<std::invoke_result_t<F, decltype(std::move(error()))>>;
    static_assert(!std::is_void_v<G>, "F has to return an error value");
    static_assert(!std::is_lvalue_reference_v<G> && std::is_rvalue_reference_v<G>,
                  "F can't return a reference type");

    if (*this) {
      return expected<void, G>{};
    } else {
      return expected<void, G>{
        unexpect,
        std::invoke(std::forward<F>(f), std::move(_storage.get_error().value()))
      };
    }
  }

private:
  impl::expected_storage<void, E> _storage;
};

} // namespace ntf
