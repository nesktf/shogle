#pragma once

namespace ntf {

template <typename T>
class singleton {
protected:
  singleton() {}

public:
  singleton(const singleton&) = delete;
  singleton& operator=(const singleton&) = delete;

public:
  static T& instance() {
    static T instance;
    return instance;
  }
};

} // namespace ntf
