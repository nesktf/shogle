#pragma once

#include <glm/vec3.hpp>

#include <functional>
#include <exception>
#include <string>

namespace ntf::shogle {

template<typename T>
using ref = std::reference_wrapper<T>;

template<typename T>
using cref = std::reference_wrapper<const T>;

struct TransformData {
  glm::vec3 pos {0.0f};
  glm::vec3 scale {0.0f};
  glm::vec3 rot {0.0f};
};

class shogle_error : public std::exception {
public:
  shogle_error(const char* _msg) :
    msg(_msg) {}
public:
  const char* what() const noexcept override {
    return msg.c_str();
  }
public:
  std::string msg;
};

} // namespace ntf::shogle
