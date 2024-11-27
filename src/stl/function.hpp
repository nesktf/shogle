#pragma once

#include "./common.hpp"

#include <functional>

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

} // namespace ntf

#ifndef SHOGLE_STL_FUNCTION_INL
#include "./function.inl"
#endif
