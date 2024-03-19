#include "settings.hpp"
#include "util.hpp"

#include "sol/sol.hpp"

namespace ntf::shogle {

Settings::Settings(int, char*[]) {
  // TODO: parse args
}

Settings::Settings(int argc, char* argv[], const char* path) :
  Settings(argc, argv) {
  const auto script = util::file_contents(path);
  if (script.empty())
    return;

  sol::state lua;
  lua.open_libraries(sol::lib::base);

  lua.script(script);

  sol::table script_settings = lua["Settings"];
  this->w_title = script_settings["w_title"];
  this->w_width = script_settings["w_width"];
  this->w_height = script_settings["w_height"];

  sol::table col_vec = script_settings["clear_color"];
  this->clear_color = glm::vec3{
    col_vec["R"],
    col_vec["G"],
    col_vec["B"]
  };
}

} // ntf::shogle
