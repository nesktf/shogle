#pragma once

namespace ntf {

template <typename T>
class Singleton {
protected:
  Singleton() {}

public:
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;

public:
  static T& instance() {
    static T instance;
    return instance;
  }
};

} // namespace ntf
