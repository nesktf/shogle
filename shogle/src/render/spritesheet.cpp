#include <shogle/render/res/spritesheet.hpp>

#include <shogle/core/log.hpp>

#include <nlohmann/json.hpp>

#include <fstream>

namespace ntf::render {

Spritesheet::Spritesheet(const Spritesheet::data_t* data) :
  Texture(data->tex_data.get()),
  sprites(std::move(data->sprites)) {}

} // namespace ntf::render
