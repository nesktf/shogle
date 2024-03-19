#include "shogle.hpp"
#include "log.hpp"

#include "level/test_level.hpp"

using namespace ntf::shogle;

int main(int argc, char* argv[]) {
  log::set_level(log::LogLevel::LOG_DEBUG);

  auto& shogle = Engine::instance();
  if (shogle.init(Settings{argc, argv, "res/script/settings.lua"}, TestLevel::create)) {
    shogle.start();
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

