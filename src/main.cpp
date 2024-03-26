#include "shogle.hpp"
#include "log.hpp"

#include "level/test_level.hpp"

using namespace ntf::shogle;

int main(int argc, char* argv[]) {
  Log::set_level(LogLevel::LOG_VERBOSE);

  auto& shogle = Engine::instance();
  if (shogle.init(Settings{argc, argv, "script/default_settings.lua"})) {
    shogle.start(TestLevel::create);
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

