#pragma once

#include <cstring>
#include <cassert>
#include <functional>
#include <cstdint>
#include <memory>
#include <iostream>

namespace ntf {

template<typename Signature,
  std::size_t buffer_sz = 2*sizeof(void*)/sizeof(char)>
class inplace_function;

template<std::size_t buffer_sz, typename R, typename... Args>
class inplace_function<R(Args...), buffer_sz> {
public:
  static constexpr std::size_t BUFFER_SIZE = buffer_sz;

private:
  using destroy_ptr = void(*)(void*);
  using invoke_ptr = R(*)(void*, Args...);
  using copyc_ptr = void(*)(void*, const void*);
  using movec_ptr = void(*)(void*, void*);

  template<typename T>
  struct function_helper{
    static void copyc(T* obj, const T* other) {
      new (obj) T{*other};
    }
    static void movec(T* obj, T* other) {
      new (obj) T{std::move(*other)};
    }
    static void destroy(T* callable) {
      callable->~T();
    }
    static R invoke(T* callable, Args... args) {
      return (*callable)(std::forward<Args>(args)...);
    }
  };

public:
  inplace_function() = default;

  template<typename T>
  inplace_function(T&& callable) :
    _destroy(reinterpret_cast<destroy_ptr>(function_helper<T>::destroy)),
    _copyc(reinterpret_cast<copyc_ptr>(function_helper<T>::copyc)),
    _movec(reinterpret_cast<movec_ptr>(function_helper<T>::movec)),
    _invoke(reinterpret_cast<invoke_ptr>(function_helper<T>::invoke)) {
    static_assert(sizeof(T) <= BUFFER_SIZE, "Size is not enough!");
    _create_inplace(std::forward<T>(callable));
  }

  ~inplace_function() noexcept {
    if (valid()) {
      _destroy_inplace();
    }
  }

  inplace_function(const inplace_function& other) :
    _destroy(other._destroy),
    _copyc(other._copyc),
    _movec(other._movec),
    _invoke(other._invoke) {
    if (other.valid()) {
      std::memset(_buf, 0, BUFFER_SIZE);
      _copyc(_buf, other._buf);
    }
  }

  inplace_function& operator=(const inplace_function& other) {
    if (valid()) {
      _destroy_inplace();
    }

    _destroy = other._destroy;
    _copyc = other._copyc;
    _movec = other._movec;
    _invoke = other._invoke;

    if (other.valid()) {
      std::memset(_buf, 0, BUFFER_SIZE);
      _copyc(_buf, other._buf);
    }

    return *this;
  }

  inplace_function(inplace_function&& other) :
    _destroy(std::move(other._destroy)),
    _copyc(std::move(other._copyc)),
    _movec(std::move(other._movec)),
    _invoke(std::move(other._invoke)) {
    if (other.valid()) {
      std::memset(_buf, 0, BUFFER_SIZE);
      _movec(_buf, other._buf);
    }
  }

  inplace_function& operator=(inplace_function&& other) {
    if (valid()) {
      _destroy_inplace();
    }

    _destroy = std::move(other._destroy);
    _copyc = std::move(other._copyc);
    _movec = std::move(other._movec);
    _invoke = std::move(other._invoke);

    if (other.valid()) {
      std::memset(_buf, 0, BUFFER_SIZE);
      _movec(_buf, other._buf);
    }

    return *this;
  }

  template<typename T>
  inplace_function& operator=(T&& callable) {
    if (valid()) {
      _destroy_inplace();
    }

    _destroy = reinterpret_cast<destroy_ptr>(function_helper<T>::destroy);
    _copyc = reinterpret_cast<copyc_ptr>(function_helper<T>::copyc);
    _movec = reinterpret_cast<movec_ptr>(function_helper<T>::movec);
    _invoke = reinterpret_cast<invoke_ptr>(function_helper<T>::invoke);

    _create_inplace(std::forward<T>(callable));

    return *this;
  }

public:
  R operator()(Args... args) {
    assert(_invoke);
    return _invoke(_buf, std::forward<Args>(args)...);
  }

public:
  bool valid() const { return _destroy != nullptr; }
  explicit operator bool() const { return valid(); }

private:
  template<typename T>
  void _create_inplace(T&& callable) {
    std::memset(_buf, 0, BUFFER_SIZE);
    if constexpr (std::is_rvalue_reference_v<T>) {
      _movec(_buf, std::addressof(callable));
    } else {
      _copyc(_buf, std::addressof(callable));
    }
  }

  void _destroy_inplace() {
    assert(_destroy);
    _destroy(_buf);
  }

private:
  destroy_ptr _destroy{nullptr};
  copyc_ptr _copyc{nullptr};
  movec_ptr _movec{nullptr};
  invoke_ptr _invoke{nullptr};
  uint8_t _buf[BUFFER_SIZE];
};

} // namespace ntf
