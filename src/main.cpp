#include <iostream>
#include "core/logger.hpp"

int main(int argc, char* argv[]) {
  using namespace ntf::shogle;
  logger::set_level(logger::LogLevel::LOG_DEBUG);

  logger::debug("henlo");
}

