#pragma once

#include "./types.hpp"

namespace ntf {

template<typename Signature, std::size_t buffer = 2*sizeof(void*)> // 2 ptrs
class inplace_function;

template<std::size_t buffer, typename R, typename... Args>
class inplace_function<R(Args...), buffer> {
public:
  using signature = R(Args...);
  using return_type = R;
  using args_type = std::tuple<Args...>;

  static constexpr std::size_t buffer_size = buffer;

private:
  using copyc_ptr = void(*)(void*, const void*);
  using movec_ptr = void(*)(void*, void*);
  using destroy_ptr = void(*)(void*);
  using invoke_ptr = R(*)(void*, Args...);

  template<typename T>
  struct function_helper {
    static void copyc(T* obj, const T* other);
    static void movec(T* obj, T* other);
    static void destroy(T* callable);
    static R invoke(T* callable, Args... args);
  };

public:
  inplace_function();

  template<typename T>
  inplace_function(T&& callable)
  noexcept(is_nothrow_forward_constructible<T>);

public:
  template<typename T>
  inplace_function& operator=(T&& callable)
  noexcept(is_nothrow_forward_constructible<T>);

  R operator()(Args... args);

public:
  bool valid() const { return _destroy != nullptr; }
  explicit operator bool() const { return valid(); }

private:
  template<typename T>
  void _create_inplace(T&& callable);

private:
  destroy_ptr _destroy{nullptr};
  copyc_ptr _copyc{nullptr};
  movec_ptr _movec{nullptr};
  invoke_ptr _invoke{nullptr};
  uint8_t _buf[buffer_size];

public:
  NTF_DECLARE_MOVE_COPY(inplace_function);
};

template<std::size_t buffer, typename R, typename... Args>
template<typename T>
void inplace_function<R(Args...), buffer>::function_helper<T>::copyc(T* obj, const T* other) {
  new (obj) T{*other};
}

template<std::size_t buffer, typename R, typename... Args>
template<typename T>
void inplace_function<R(Args...), buffer>::function_helper<T>::movec(T* obj, T* other) {
  new (obj) T{std::move(*other)};
}

template<std::size_t buffer, typename R, typename... Args>
template<typename T>
void inplace_function<R(Args...), buffer>::function_helper<T>::destroy(T* obj) {
  obj->~T();
}

template<std::size_t buffer, typename R, typename... Args>
template<typename T>
R inplace_function<R(Args...), buffer>::function_helper<T>::invoke(T* obj, Args... args) {
  return std::invoke(*obj, std::forward<Args>(args)...);
}


template<std::size_t buffer, typename R, typename... Args>
inplace_function<R(Args...), buffer>::inplace_function() { std::memset(_buf, 0, buffer_size); }

template<std::size_t buffer, typename R, typename... Args>
template<typename T>
inplace_function<R(Args...), buffer>::inplace_function(T&& callable)
  noexcept(is_nothrow_forward_constructible<T>) :
  _destroy(reinterpret_cast<destroy_ptr>(function_helper<T>::destroy)),
  _copyc(reinterpret_cast<copyc_ptr>(function_helper<T>::copyc)),
  _movec(reinterpret_cast<movec_ptr>(function_helper<T>::movec)),
  _invoke(reinterpret_cast<invoke_ptr>(function_helper<T>::invoke)) {

  static_assert(sizeof(T) <= buffer_size, "Size is not enough!");
  _create_inplace(std::forward<T>(callable));
}


template<std::size_t buffer, typename R, typename... Args>
template<typename T>
auto inplace_function<R(Args...), buffer>::operator=(T&& callable)
  noexcept(is_nothrow_forward_constructible<T>) -> inplace_function& {

  static_assert(sizeof(T) <= buffer_size, "Size is not enough!");

  if (valid()) {
    std::invoke(_destroy, _buf);
  }

  _destroy = reinterpret_cast<destroy_ptr>(function_helper<T>::destroy);
  _copyc = reinterpret_cast<copyc_ptr>(function_helper<T>::copyc);
  _movec = reinterpret_cast<movec_ptr>(function_helper<T>::movec);
  _invoke = reinterpret_cast<invoke_ptr>(function_helper<T>::invoke);

  _create_inplace(std::forward<T>(callable));

  return *this;
}

template<std::size_t buffer, typename R, typename... Args>
R inplace_function<R(Args...), buffer>::operator()(Args... args) {
  NTF_ASSERT(_invoke);
  return std::invoke(_invoke, _buf, std::forward<Args>(args)...);
}


template<std::size_t buffer, typename R, typename... Args>
template<typename T>
void inplace_function<R(Args...), buffer>::_create_inplace(T&& callable) {
  std::memset(_buf, 0, buffer_size);
  if constexpr (std::is_rvalue_reference_v<T>) {
    std::invoke(_movec, _buf, std::addressof(callable));
  } else {
    std::invoke(_copyc, _buf, std::addressof(callable));
  }
}


template<std::size_t buffer, typename R, typename... Args>
inplace_function<R(Args...), buffer>::~inplace_function() noexcept {
  if (valid()) {
    std::invoke(_destroy, _buf);
  }
}

template<std::size_t buffer, typename R, typename... Args>
inplace_function<R(Args...), buffer>::inplace_function(const inplace_function& f) noexcept :
  _destroy(f._destroy),
  _copyc(f._copyc),
  _movec(f._movec),
  _invoke(f._invoke) {
  std::memset(_buf, 0, buffer_size);
  if (f.valid()) {
    std::invoke(_copyc, _buf, f._buf);
  }
}

template<std::size_t buffer, typename R, typename... Args>
inplace_function<R(Args...), buffer>::inplace_function(inplace_function&& f) noexcept :
  _destroy(std::move(f._destroy)),
  _copyc(std::move(f._copyc)),
  _movec(std::move(f._movec)),
  _invoke(std::move(f._invoke)) {
  std::memset(_buf, 0, buffer_size);
  if (f.valid()) {
    std::invoke(_movec, _buf, f._buf);
  }
}

template<std::size_t buffer, typename R, typename... Args>
auto inplace_function<R(Args...), buffer>::operator=(const inplace_function& f) noexcept
                                                                             -> inplace_function& {
  if (valid()) {
    std::invoke(_destroy, _buf);
  }

  _destroy = f._destroy;
  _copyc = f._copyc;
  _movec = f._movec;
  _invoke = f._invoke;

  if (f.valid()) {
    std::memset(_buf, 0, buffer_size);
    std::invoke(_copyc, _buf, f._buf);
  }

  return *this;
}

template<std::size_t buffer, typename R, typename... Args>
auto inplace_function<R(Args...), buffer>::operator=(inplace_function&& f) noexcept
                                                                             -> inplace_function& {
  if (valid()) {
    std::invoke(_destroy, _buf);
  }

  _destroy = std::move(f._destroy);
  _copyc = std::move(f._copyc);
  _movec = std::move(f._movec);
  _invoke = std::move(f._invoke);

  if (f.valid()) {
    std::memset(_buf, 0, buffer_size);
    std::invoke(_movec, _buf, f._buf);
  }

  return *this;
}

} // namespace ntf
