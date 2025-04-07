#pragma once

#include "./types.hpp"

namespace ntf {

template<auto fun, typename T>
constexpr auto lambda_wrap(T& obj) {
  return [&obj](auto&&... args) {
    return (&obj->*fun)(std::forward<decltype(args)>(args)...);
  };
}

template<auto fun, typename T>
constexpr auto lambda_wrap(T* obj) {
  return [obj](auto&&... args) {
    return (obj->*fun)(std::forward<decltype(args)>(args)...);
  };
}

template<typename Signature, size_t buff_sz = 2*sizeof(void*)> // 2 ptrs
class inplace_function;

template<size_t buff_sz, typename R, typename... Args>
class inplace_function<R(Args...), buff_sz> {
private:
  enum class vtable_state {
    empty = 0,
    functor,
    funptr,
  };

  struct vtable_t {
    void (*destroy)(void*);
    void (*copy)(void*, const void*);
    R (*invoke)(void*, Args...);
  };

  template<typename T>
  static constexpr vtable_t _vtable_for {
    .destroy = +[](void* obj) -> void {
      static_assert(std::is_destructible_v<T>, "T has to be destructible");
      static_assert(std::is_nothrow_destructible_v<T>, "T has to be nothrow destructible");
      static_cast<T*>(obj)->~T();
    },
    .copy = +[](void* obj, const void* other) -> void {
      static_assert(std::copy_constructible<T>, "T has to be copy constructible");
      std::construct_at(static_cast<T*>(obj), *static_cast<const T*>(other));
    },
    .invoke_obj = +[](void* obj, Args... args) -> R {
      static_assert(std::is_invocable_r_v<R, T, Args...>, "T is not a valid callable type");
      return std::invoke(*static_cast<T*>(obj), std::forward<Args>(args)...);
    },
  };

public:
  using signature = R(Args...);
  using return_type = R;

  static constexpr size_t BUFFER_SIZE = buff_sz;

public:
  constexpr inplace_function() noexcept :
    _state{vtable_state::empty} {}

  constexpr inplace_function(std::nullptr_t) noexcept :
    _state{vtable_state::empty} {}

  constexpr inplace_function(R(*fun)(Args...)) noexcept :
    _state{vtable_state::funptr}, _fun_invoke{fun} {}

  template<typename T>
  requires(sizeof(T) <= BUFFER_SIZE)
  constexpr inplace_function(T&& obj)
  noexcept(is_nothrow_forward_constructible<T>) :
    _state{vtable_state::functor}, _vtable{&_vtable_for<std::remove_cvref_t<T>>}
  {
    _construct<std::remove_cvref_t<T>>(std::forward<T>(obj));
  }

  template<typename T, typename... CArgs>
  requires(sizeof(T) <= BUFFER_SIZE)
  constexpr explicit inplace_function(std::in_place_type_t<T>, CArgs&&... args)
  noexcept(std::is_nothrow_constructible_v<T, CArgs...>) :
    _state{vtable_state::functor}, _vtable{&_vtable_for<std::remove_cvref_t<T>>}
  {
    _construct<std::remove_cvref_t<T>>(std::forward<CArgs>(args)...);
  }

  template<typename T, typename U, typename... CArgs>
  requires(sizeof(T) <= BUFFER_SIZE)
  constexpr explicit inplace_function(std::in_place_type_t<T>,
                                      std::initializer_list<U> il, CArgs&&... args)
  noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, CArgs...>) :
    _state{vtable_state::functor}, _vtable{&_vtable_for<std::remove_cvref_t<T>>}
  {
    _construct<std::remove_cvref_t<T>>(il, std::forward<CArgs>(args)...);
  }
  
public:
  constexpr ~inplace_function() noexcept { _destroy(); }

  constexpr inplace_function(const inplace_function& other) :
    _state{other._state}
  {
    if (other._state == vtable_state::funptr) {
      _fun_invoke = other._fun_invoke;
      return;
    } else if (other._state == vtable_state::functor) {
      _vtable = other._vtable;
      _copy(std::addressof(other._buf));
    }
  }

  constexpr inplace_function(inplace_function&& other) noexcept :
    _state{std::move(other._state)}
  {
    if (other._state == vtable_state::funptr) {
      _fun_invoke = std::move(other._fun_invoke);
    } else if (other._state == vtable_state::functor) {
      _vtable = std::move(other._vtable);
      _move(std::addressof(other._buf));
    }

    other._state = vtable_state::empty;
  }

public:
  constexpr R operator()(Args... args) {
    if (_state == vtable_state::funptr) {
      return std::invoke(_fun_invoke, std::forward<Args>(args)...);
    } else {
      return std::invoke(_vtable->invoke,
                         static_cast<void*>(std::addressof(_buf)),
                         std::forward<Args>(args)...);
    }
  }

public:
  constexpr inplace_function& operator=(std::nullptr_t) noexcept {
    _destroy();
    _state = vtable_state::empty;
    return *this;
  }

  template<typename T>
  requires(sizeof(T) <= BUFFER_SIZE)
  constexpr inplace_function& operator=(T&& obj)
  noexcept(is_nothrow_forward_constructible<T>)
  {
    _destroy();

    _state = vtable_state::functor;
    _vtable = &_vtable_for<std::remove_cvref_t<T>>;
    _construct<std::remove_cvref_t<T>>(std::forward<T>(obj));

    return *this;
  }

  constexpr inplace_function& operator=(R(*fun)(Args...)) noexcept {
    _destroy();

    _state = vtable_state::funptr;
    _fun_invoke = fun;

    return *this;
  }

  constexpr inplace_function& operator=(const inplace_function& other) {
    if (std::addressof(other) == this) {
      return *this;
    }

    _destroy();

    _state = other._state;
    if (other._state == vtable_state::funptr) {
      _fun_invoke = other._fun_invoke;
    } else if (other._state == vtable_state::functor) {
      _vtable = other._vtable;
      _copy(std::addressof(other._buf));
    }

    return *this;
  }

  constexpr inplace_function& operator=(inplace_function&& other) noexcept {
    if (std::addressof(other) == this) {
      return *this;
    }

    _destroy();

    _state = std::move(other._state);
    if (other._state == vtable_state::funptr) {
      _fun_invoke = std::move(other._fun_invoke);
    } else if (other._state == vtable_state::functor) {
      _vtable = std::move(other._vtable);
      _move(std::addressof(other._buf));
    }

    other._state = vtable_state::empty;

    return *this;
  }

  constexpr friend bool operator==(const inplace_function& f, std::nullptr_t) noexcept {
    return f.is_empty();
  }

public:
  constexpr bool is_empty() const { return _state == vtable_state::empty; }
  constexpr explicit operator bool() const { return !is_empty(); }

private:
  template<typename T, typename... CArgs>
  constexpr void _construct(CArgs&&... args) {
    std::construct_at(static_cast<T*>(_buf), std::forward<CArgs>(args)...);
  }

  constexpr void _copy(const uint8* other) {
    std::invoke(_vtable->copy,
                static_cast<void*>(std::addressof(_buf)),
                static_cast<const void*>(other));
  }

  constexpr void _move(uint8* other) {
    std::memcpy(std::addressof(_buf), other, BUFFER_SIZE);
  }

  constexpr void _destroy() noexcept {
    if (_state != vtable_state::functor) {
      return;
    }
    std::invoke(_vtable->destroy, static_cast<void*>(_buf));
  }

private:
  uint8 _buf[BUFFER_SIZE];
  vtable_state _state;
  union {
    const vtable_t* _vtable;
    R (*_fun_invoke)(Args...);
  };
};

} // namespace ntf
