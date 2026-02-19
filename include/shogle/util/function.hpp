#pragma once

#include <shogle/core.hpp>

#include <functional>

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
  explicit constexpr fn_ref(func_ptr_type func) noexcept : _data(nullptr), _func_invoke(func) {}

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
  explicit constexpr fn_ref(func_ptr_type func) noexcept : _data(nullptr), _func_invoke(func) {}

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

} // namespace shogle
