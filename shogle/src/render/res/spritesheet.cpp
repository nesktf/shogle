#include <shogle/res/spritesheet.hpp>

#include <shogle/core/log.hpp>

#include <nlohmann/json.hpp>

#include <fstream>

namespace ntf {

SpritesheetData::SpritesheetData(std::string path) {
  using json = nlohmann::json;

  std::ifstream f{path};
  json data = json::parse(f);

  std::string dir = path.substr(0, path.find_last_of('/'));

  tex_data = make_uptr<TextureData>(dir+"/"+data["file"].template get<std::string>());

  auto content = data["content"];
  for (auto& sprite : content) {
    SpriteData sp_data{};

    std::string name = sprite["name"].template get<std::string>();

    sp_data.count = sprite["count"].template get<size_t>();
    sp_data.x0 = sprite["x0"].template get<size_t>();
    sp_data.y0 = sprite["y0"].template get<size_t>();
    sp_data.dx = sprite["dx"].template get<size_t>();
    sp_data.dy = sprite["dy"].template get<size_t>();
    sp_data.x = static_cast<size_t>(tex_data->width);
    sp_data.y = static_cast<size_t>(tex_data->height);
    sp_data.cols = sprite["cols"].template get<size_t>();
    sp_data.rows = std::ceil((float)sp_data.count/(float)sp_data.cols);

    sprites.emplace(std::make_pair(name, std::move(sp_data)));
  }
}

Spritesheet::Spritesheet(const Spritesheet::data_t* data) :
  Texture(data->tex_data.get()),
  sprites(std::move(data->sprites)) {}

} // namespace ntf
