#pragma once

#include <string>
#include <exception>

namespace ntf {

class error : public std::exception {
public:
  error(const char* _msg) :
    msg(_msg) {}

  error(std::string _msg) :
    msg(_msg) {}

public:
  const char* what() const noexcept override {
    return msg.c_str();
  }

protected:
  std::string msg;
};

}
