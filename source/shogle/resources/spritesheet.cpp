#include <shogle/resources/spritesheet.hpp>

#include <shogle/resources/util.hpp>

#include <nlohmann/json.hpp>

#include <fstream>

namespace ntf::shogle::resources {

spritesheet_data::spritesheet_data(std::string _path) :
  path(std::move(_path)) {
  using json = nlohmann::json;

  std::ifstream f{path};
  json data = json::parse(f);

  auto tex_path = file_dir(path)+"/"+data["file"].template get<std::string>();
  texture = texture2d_data{std::move(tex_path)};

  auto content = data["content"];
  for (auto& curr_sprite : content) {
    sprite_data sp_data{};

    sp_data.name = curr_sprite["name"].template get<std::string>();
    sp_data.count = curr_sprite["count"].template get<size_t>();
    sp_data.x0 = curr_sprite["x0"].template get<size_t>();
    sp_data.y0 = curr_sprite["y0"].template get<size_t>();
    sp_data.dx = curr_sprite["dx"].template get<size_t>();
    sp_data.dy = curr_sprite["dy"].template get<size_t>();
    sp_data.x = static_cast<size_t>(texture.width);
    sp_data.y = static_cast<size_t>(texture.height);
    sp_data.cols = curr_sprite["cols"].template get<size_t>();
    sp_data.rows = std::ceil((float)sp_data.count/(float)sp_data.cols);

    sprites.emplace_back(std::move(sp_data));
  }
}

spritesheet::spritesheet(std::string path) :
  spritesheet(data_t{std::move(path)}) {}

spritesheet::spritesheet(data_t data) :
  _path(std::move(data.path)),
  _texture(std::move(data.texture)) {
  
  for (auto& spr_data : data.sprites) {
    sprite sprite{spr_data.count};

    sprite.texture = &_texture;
    sprite.corrected_scale.x =
      (float)(spr_data.dx*spr_data.rows) / (float)(spr_data.dy*spr_data.cols);
    sprite.corrected_scale.y = 1.0f;

    sprite.linear_offset.x = 
      (float)(spr_data.dx) / (float)(spr_data.x*spr_data.cols);
    sprite.linear_offset.y =
      (float)(spr_data.dy) / (float)(spr_data.y*spr_data.rows);

    for (size_t i = 0; i < spr_data.count; ++i) {
      size_t row = i / spr_data.cols;
      size_t col = i % spr_data.cols;

      vec2 frac_a{
        spr_data.x0 + (col*spr_data.dx),
        spr_data.y0 + (row*spr_data.dy)
      };
      vec2 frac_b{
        spr_data.x*spr_data.cols,
        spr_data.y*spr_data.rows
      };

      sprite.const_offset[i].x = frac_a.x / frac_b.x;
      sprite.const_offset[i].y = frac_a.y / frac_b.y;
    }

    _sprites.emplace(std::make_pair(std::move(spr_data.name), std::move(sprite)));
  }

  // "sprite" for the whole spritesheet
  sprite __sheet{1};
  __sheet.corrected_scale = vec2{1.0f};
  __sheet.linear_offset = vec2{1.0f};
  __sheet.const_offset[0] = vec2{0.0f};
  __sheet.texture = &_texture;
  _sprites.emplace(std::make_pair(std::string{"__sheet"}, std::move(__sheet)));
}

} // namespace ntf::shogle::resources
