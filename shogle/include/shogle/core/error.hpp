#pragma once

#include <fmt/format.h>

#include <string>
#include <exception>

namespace ntf {

class error : public std::exception {
public:
  template<typename... Args>
  error(fmt::format_string<Args...> format, Args&&... args) :
    msg(fmt::format(format, std::forward<Args>(args)...)) {}

public:
  const char* what() const noexcept override {
    return msg.c_str();
  }

protected:
  std::string msg;
};

}
