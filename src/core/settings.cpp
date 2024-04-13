#include "core/settings.hpp"
#include "core/log.hpp"

#include "util/fs.hpp"

#include "sol/sol.hpp"

namespace ntf {

Settings::Settings(int, char*[]) {
  // TODO: parse args
}

Settings::Settings(int argc, char* argv[], const char* path) :
  Settings(argc, argv) {
  const auto script = fs::file_contents(path);
  if (script.empty())
    return;

  sol::state lua;
  lua.open_libraries(sol::lib::base);

  lua.script(script);

  sol::table script_settings = lua["settings"];
  this->w_title = script_settings["w_title"];
  this->w_width = script_settings["w_width"];
  this->w_height = script_settings["w_height"];

  sol::table col_vec = script_settings["clear_color"];
  this->clear_color = color3{
    col_vec["R"],
    col_vec["G"],
    col_vec["B"]
  };
  Log::info("[Settings] Settings loaded (file: {})", path);
}

} // ntf
