#pragma once

#include "./types.hpp"

namespace ntf {

template <typename T>
class singleton {
protected:
  singleton() noexcept {}

private:
  static auto& _instance() {
    static struct holder {
      holder() noexcept :
        dummy{}, inited{false} {}
      ~holder() noexcept {}

      union {
        T obj;
        char dummy;
      };
      bool inited;
    } h;
    return h;
  }

public:
  template<typename... Args>
  requires(std::constructible_from<T, Args...>)
  static T& construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
    auto& storage = _instance();
    NTF_ASSERT(!storage.inited);
    std::construct_at(&storage.obj, std::forward<Args>(args)...);
    storage.inited = true;
    return storage.obj;
  }

  static void destroy() noexcept(std::is_nothrow_destructible_v<T>) {
    auto& storage = _instance();
    NTF_ASSERT(storage.inited);
    storage.obj.~T();
    storage.inited = false;
  }

  static T& instance() {
    auto& storage = _instance();
    NTF_ASSERT(storage.inited);
    return storage.obj;
  }

public:
  singleton(const singleton&) = delete;
  singleton(singleton&&) = delete;
  singleton& operator=(const singleton&) = delete;
  singleton& operator=(singleton&&) = delete;
};

template <typename T>
class lazy_singleton {
protected:
  lazy_singleton() noexcept {}

public:
  static T& instance() {
    static T instance;
    return instance;
  }

public:
  lazy_singleton(const lazy_singleton&) = delete;
  lazy_singleton(lazy_singleton&&) = delete;
  lazy_singleton& operator=(const lazy_singleton&) = delete;
  lazy_singleton& operator=(lazy_singleton&&) = delete;
};

} // namespace ntf
