#pragma once

#include <cstddef>
#include <shogle/core.hpp>

#include <functional>
#include <type_traits>

namespace shogle {

namespace impl {

template<typename T, bool IsConst, bool IsNoexcept, typename Ret, typename... Args>
struct erased_invoker {
  static constexpr Ret invoke(void* ptr, Args... args) noexcept(IsNoexcept) {
    if constexpr (std::is_void_v<Ret>) {
      std::invoke(*static_cast<T*>(ptr), std::forward<Args>(args)...);
    } else {
      return std::invoke(*static_cast<T*>(ptr), std::forward<Args>(args)...);
    }
  }
};

template<typename T, bool IsNoexcept, typename Ret, typename... Args>
struct erased_invoker<T, true, IsNoexcept, Ret, Args...> {
  static constexpr Ret invoke(const void* ptr, Args... args) noexcept(IsNoexcept) {
    if constexpr (std::is_void_v<Ret>) {
      std::invoke(*static_cast<const T*>(ptr), std::forward<Args>(args)...);
    } else {
      return std::invoke(*static_cast<const T*>(ptr), std::forward<Args>(args)...);
    }
  }
};

} // namespace impl

template<typename Signature>
class fn_ref;

template<bool IsNoexcept, typename Ret, typename... Args>
class fn_ref<Ret(Args...) const noexcept(IsNoexcept)> {
public:
  using signature = Ret(Args...) const noexcept(IsNoexcept);
  using return_type = Ret;
  using func_ptr_type = Ret (*)(Args...) noexcept(IsNoexcept);

private:
  using functor_ptr_type = Ret (*)(const void*, Args...) noexcept(IsNoexcept);

public:
  explicit constexpr fn_ref(func_ptr_type func) : _data(nullptr), _func_invoke(func) {
    SHOGLE_ASSERT(func != nullptr);
  }

  template<typename T>
  requires(std::is_invocable_r_v<Ret, T, Args...> && !std::is_same_v<fn_ref, T>)
  constexpr fn_ref(const T& functor) noexcept :
      _data(static_cast<const void*>(std::addressof(functor))),
      _functor_invoke(&impl::erased_invoker<T, true, IsNoexcept, Ret, Args...>::invoke) {}

public:
  constexpr ~fn_ref() noexcept = default;
  constexpr fn_ref(const fn_ref&) noexcept = default;
  constexpr fn_ref(fn_ref&&) noexcept = default;

public:
  constexpr Ret operator()(Args... args) const noexcept(IsNoexcept) {
    if (_data) {
      if constexpr (std::is_void_v<Ret>) {
        std::invoke(_functor_invoke, _data, std::forward<Args>(args)...);
      } else {
        return std::invoke(_functor_invoke, _data, std::forward<Args>(args)...);
      }
    } else {
      if constexpr (std::is_void_v<Ret>) {
        std::invoke(_func_invoke, std::forward<Args>(args)...);
      } else {
        return std::invoke(_func_invoke, std::forward<Args>(args)...);
      }
    }
  }

public:
  constexpr fn_ref& operator=(func_ptr_type func) noexcept {
    _data = nullptr;
    _func_invoke = func;
    return *this;
  }

  template<typename T>
  requires(std::is_invocable_r_v<Ret, T, Args...> && !std::is_same_v<fn_ref, T>)
  constexpr fn_ref& operator=(const T& functor) noexcept {
    _data = static_cast<const void*>(std::addressof(functor));
    _functor_invoke = &impl::erased_invoker<T, true, IsNoexcept, Ret, Args...>::invoke;
    return *this;
  }

  constexpr fn_ref& operator=(const fn_ref&) noexcept = default;
  constexpr fn_ref& operator=(fn_ref&&) noexcept = default;

private:
  const void* _data;

  union {
    func_ptr_type _func_invoke;
    functor_ptr_type _functor_invoke;
  };
};

template<bool IsNoexcept, typename Ret, typename... Args>
class fn_ref<Ret(Args...) noexcept(IsNoexcept)> {
public:
  using signature = Ret(Args...) noexcept(IsNoexcept);
  using return_type = Ret;
  using func_ptr_type = Ret (*)(Args...) noexcept(IsNoexcept);

private:
  using functor_ptr_type = Ret (*)(void*, Args...) noexcept(IsNoexcept);

public:
  explicit constexpr fn_ref(func_ptr_type func) : _data(nullptr), _func_invoke(func) {
    SHOGLE_ASSERT(func != nullptr);
  }

  template<typename T>
  requires(std::is_invocable_r_v<Ret, T, Args...> && !std::is_same_v<fn_ref, T>)
  constexpr fn_ref(T& functor) noexcept :
      _data(static_cast<void*>(std::addressof(functor))),
      _functor_invoke(&impl::erased_invoker<T, false, IsNoexcept, Ret, Args...>::invoke) {}

public:
  constexpr ~fn_ref() noexcept = default;
  constexpr fn_ref(const fn_ref&) noexcept = default;
  constexpr fn_ref(fn_ref&&) noexcept = default;

public:
  constexpr Ret operator()(Args... args) const noexcept(IsNoexcept) {
    if (_data) {
      if constexpr (std::is_void_v<Ret>) {
        std::invoke(_functor_invoke, _data, std::forward<Args>(args)...);
      } else {
        return std::invoke(_functor_invoke, _data, std::forward<Args>(args)...);
      }
    } else {
      if constexpr (std::is_void_v<Ret>) {
        std::invoke(_func_invoke, std::forward<Args>(args)...);
      } else {
        return std::invoke(_func_invoke, std::forward<Args>(args)...);
      }
    }
  }

  constexpr fn_ref& operator=(func_ptr_type func) noexcept {
    _data = nullptr;
    _func_invoke = func;
    return *this;
  }

  template<typename T>
  requires(std::is_invocable_r_v<Ret, T, Args...> && !std::is_same_v<fn_ref, T>)
  constexpr fn_ref& operator=(T& functor) noexcept {
    _data = static_cast<void*>(std::addressof(functor));
    _functor_invoke = &impl::erased_invoker<T, false, IsNoexcept, Ret, Args...>::invoke;
    return *this;
  }

  constexpr fn_ref& operator=(const fn_ref&) noexcept = default;
  constexpr fn_ref& operator=(fn_ref&&) noexcept = default;

private:
  void* _data;

  union {
    func_ptr_type _func_invoke;
    functor_ptr_type _functor_invoke;
  };
};

template<typename Signature, size_t MaxSize, size_t MaxAlign = alignof(std::max_align_t)>
class inplace_trivial_fn;

template<size_t MaxSize, size_t MaxAlign, bool IsNoexcept, typename Ret, typename... Args>
class inplace_trivial_fn<Ret(Args...) const noexcept(IsNoexcept), MaxSize, MaxAlign> {
public:
  using func_ptr_type = Ret (*)(Args...) noexcept(IsNoexcept);

private:
  using functor_ptr_type = Ret (*)(const void*, Args...) noexcept(IsNoexcept);

  template<typename T>
  static constexpr bool is_valid_func =
    std::is_invocable_r_v<Ret, std::remove_cvref_t<T>, Args...> &&
    !std::is_same_v<inplace_trivial_fn, std::remove_cvref_t<T>> &&
    std::is_trivially_copyable_v<std::remove_cvref_t<T>> &&
    std::is_trivially_destructible_v<std::remove_cvref_t<T>>;

public:
  explicit inplace_trivial_fn(func_ptr_type func) : _func_invoke(func), _is_functor(false) {
    SHOGLE_ASSERT(func != nullptr);
  }

  template<typename F>
  requires(is_valid_func<F>)
  inplace_trivial_fn(F&& functor) noexcept :
      _functor_invoke(
        &impl::erased_invoker<std::remove_cvref_t<F>, true, IsNoexcept, Ret, Args...>::invoke),
      _is_functor(true) {
    std::construct_at(_buffer_as<std::remove_cvref_t<F>>(), std::forward<F>(functor));
  }

  template<typename F, typename... Args2>
  requires(is_valid_func<F> && std::constructible_from<F, Args2...>)
  inplace_trivial_fn(std::in_place_type_t<F>, Args2&&... args) :
      _functor_invoke(&impl::erased_invoker<F, true, IsNoexcept, Ret, Args...>::invoke),
      _is_functor(true) {
    std::construct_at(_buffer_as<F>(), std::forward<Args2>(args)...);
  }

public:
  inplace_trivial_fn(const inplace_trivial_fn&) noexcept = default;
  inplace_trivial_fn(inplace_trivial_fn&&) noexcept = default;
  ~inplace_trivial_fn() noexcept = default;

public:
  constexpr Ret operator()(Args... args) const noexcept(IsNoexcept) {
    if (_is_functor) {
      if constexpr (std::is_void_v<Ret>) {
        std::invoke(_functor_invoke, (const void*)_buffer, std::forward<Args>(args)...);
      } else {
        return std::invoke(_functor_invoke, (const void*)_buffer, std::forward<Args>(args)...);
      }
    } else {
      if constexpr (std::is_void_v<Ret>) {
        std::invoke(_func_invoke, std::forward<Args>(args)...);
      } else {
        return std::invoke(_func_invoke, std::forward<Args>(args)...);
      }
    }
  }

public:
  template<typename F>
  requires(is_valid_func<F>)
  inplace_trivial_fn& emplace(F&& functor) noexcept {
    _functor_invoke =
      &impl::erased_invoker<std::remove_cvref_t<F>, true, IsNoexcept, Ret, Args...>::invoke;
    _is_functor = true;
    std::construct_at(_buffer_as<std::remove_cvref_t<F>>(), std::forward<F>(functor));
    return *this;
  }

  template<typename F, typename... Args2>
  requires(is_valid_func<F> && std::constructible_from<F, Args2...>)
  inplace_trivial_fn& emplace(std::in_place_type_t<F>, Args2&&... args) noexcept {
    _functor_invoke =
      &impl::erased_invoker<std::remove_cvref_t<F>, true, IsNoexcept, Ret, Args...>::invoke;
    _is_functor = true;
    std::construct_at(_buffer_as<F>(), std::forward<Args2>(args)...);
    return *this;
  }

  inplace_trivial_fn& emplace(func_ptr_type func) {
    SHOGLE_ASSERT(func != nullptr);
    _func_invoke = func;
    _is_functor = false;
    return *this;
  }

public:
  inplace_trivial_fn& operator=(const inplace_trivial_fn&) noexcept = default;
  inplace_trivial_fn& operator=(inplace_trivial_fn&&) noexcept = default;

  template<typename F>
  requires(is_valid_func<F>)
  inplace_trivial_fn& operator=(F&& functor) {
    return emplace(std::forward<F>(functor));
  }

  inplace_trivial_fn& operator==(func_ptr_type func) { return emplace(func); }

private:
  template<typename T>
  T* _buffer_as() noexcept {
    return std::launder(reinterpret_cast<T*>(&_buffer[0]));
  }

private:
  alignas(MaxAlign) u8 _buffer[MaxSize];

  union {
    func_ptr_type _func_invoke;
    functor_ptr_type _functor_invoke;
  };

  bool _is_functor;
};

template<size_t MaxSize, size_t MaxAlign, bool IsNoexcept, typename Ret, typename... Args>
class inplace_trivial_fn<Ret(Args...) noexcept(IsNoexcept), MaxSize, MaxAlign> {
public:
  using func_ptr_type = Ret (*)(Args...) noexcept(IsNoexcept);

private:
  using functor_ptr_type = Ret (*)(void*, Args...) noexcept(IsNoexcept);

  template<typename T>
  static constexpr bool is_valid_func =
    std::is_invocable_r_v<Ret, std::remove_cvref_t<T>, Args...> &&
    !std::is_same_v<inplace_trivial_fn, std::remove_cvref_t<T>> &&
    std::is_trivially_copyable_v<std::remove_cvref_t<T>> &&
    std::is_trivially_destructible_v<std::remove_cvref_t<T>>;

public:
  explicit inplace_trivial_fn(func_ptr_type func) : _func_invoke(func), _is_functor(false) {
    SHOGLE_ASSERT(func != nullptr);
  }

  template<typename F>
  requires(is_valid_func<F>)
  inplace_trivial_fn(F&& functor) noexcept :
      _functor_invoke(
        &impl::erased_invoker<std::remove_cvref_t<F>, false, IsNoexcept, Ret, Args...>::invoke),
      _is_functor(true) {
    std::construct_at(_buffer_as<std::remove_cvref_t<F>>(), std::forward<F>(functor));
  }

  template<typename F, typename... Args2>
  requires(is_valid_func<F> && std::constructible_from<F, Args2...>)
  inplace_trivial_fn(std::in_place_type_t<F>, Args2&&... args) :
      _functor_invoke(&impl::erased_invoker<F, false, IsNoexcept, Ret, Args...>::invoke),
      _is_functor(true) {
    std::construct_at(_buffer_as<F>(), std::forward<Args2>(args)...);
  }

public:
  inplace_trivial_fn(const inplace_trivial_fn&) noexcept = default;
  inplace_trivial_fn(inplace_trivial_fn&&) noexcept = default;
  ~inplace_trivial_fn() noexcept = default;

public:
  constexpr Ret operator()(Args... args) noexcept(IsNoexcept) {
    if (_is_functor) {
      if constexpr (std::is_void_v<Ret>) {
        std::invoke(_functor_invoke, (void*)_buffer, std::forward<Args>(args)...);
      } else {
        return std::invoke(_functor_invoke, (void*)_buffer, std::forward<Args>(args)...);
      }
    } else {
      if constexpr (std::is_void_v<Ret>) {
        std::invoke(_func_invoke, std::forward<Args>(args)...);
      } else {
        return std::invoke(_func_invoke, std::forward<Args>(args)...);
      }
    }
  }

public:
  template<typename F>
  requires(is_valid_func<F>)
  inplace_trivial_fn& emplace(F&& functor) noexcept {
    _functor_invoke =
      &impl::erased_invoker<std::remove_cvref_t<F>, false, IsNoexcept, Ret, Args...>::invoke;
    _is_functor = true;
    std::construct_at(_buffer_as<std::remove_cvref_t<F>>(), std::forward<F>(functor));
    return *this;
  }

  template<typename F, typename... Args2>
  requires(is_valid_func<F> && std::constructible_from<F, Args2...>)
  inplace_trivial_fn& emplace(std::in_place_type_t<F>, Args2&&... args) noexcept {
    _functor_invoke =
      &impl::erased_invoker<std::remove_cvref_t<F>, false, IsNoexcept, Ret, Args...>::invoke;
    _is_functor = true;
    std::construct_at(_buffer_as<F>(), std::forward<Args2>(args)...);
    return *this;
  }

  inplace_trivial_fn& emplace(func_ptr_type func) {
    SHOGLE_ASSERT(func != nullptr);
    _func_invoke = func;
    _is_functor = false;
    return *this;
  }

public:
  inplace_trivial_fn& operator=(const inplace_trivial_fn&) noexcept = default;
  inplace_trivial_fn& operator=(inplace_trivial_fn&&) noexcept = default;

  template<typename F>
  requires(is_valid_func<F>)
  inplace_trivial_fn& operator=(F&& functor) {
    return emplace(std::forward<F>(functor));
  }

  inplace_trivial_fn& operator==(func_ptr_type func) { return emplace(func); }

private:
  template<typename T>
  T* _buffer_as() noexcept {
    return std::launder(reinterpret_cast<T*>(&_buffer[0]));
  }

private:
  alignas(MaxAlign) u8 _buffer[MaxSize];

  union {
    func_ptr_type _func_invoke;
    functor_ptr_type _functor_invoke;
  };

  bool _is_functor;
};

} // namespace shogle
