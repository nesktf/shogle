#include <iostream>
#include "core/logger.hpp"
#include "core/game_state.hpp"

int main(int argc, char* argv[]) {
  using namespace ntf::shogle;
  logger::set_level(logger::LogLevel::LOG_DEBUG);

  auto& state = GameState::instance();
  state.init(800, 600, "shogle", argc, argv);

  while (state.main_loop())
    ;;
}

