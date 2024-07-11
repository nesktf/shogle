#include <shogle/res/spritesheet.hpp>

#include <shogle/res/util.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

#include <nlohmann/json.hpp>

#include <fstream>

using json = nlohmann::json;

namespace ntf::shogle {

sprite::sprite(texture2d& tex, vec2 scale, vec2 lin_off, std::vector<vec2> con_off) :
  _texture(&tex), _corrected_scale(scale),
  _linear_offset(lin_off), _const_offset(std::move(con_off)) {};

spritesheet_data::spritesheet_data(std::string_view path_, tex_filter filter, tex_wrap wrap) {
  std::ifstream f{path_.data()};
  json data = json::parse(f);

  auto texture_path = file_dir(path_.data())+"/"+data["file"].template get<std::string>();
  texture = texture2d_data{texture_path, filter, wrap};

  auto sprite_content = data["content"];
  sprites.resize(sprite_content.size());
  for (size_t i = 0; i < sprite_content.size(); ++i) {
    auto& sprite = sprite_content[i];

    size_t count = sprite["count"].template get<size_t>();
    size_t cols = sprite["cols"].template get<size_t>();
    float row_ratio = static_cast<float>(count)/static_cast<float>(cols);
    sprites[i] = sprite_data{
      .name = sprite["name"].template get<std::string>(),
      .count = count,
      .x = static_cast<size_t>(texture.width),
      .y = static_cast<size_t>(texture.height),
      .x0 = sprite["x0"].template get<size_t>(),
      .y0 = sprite["y0"].template get<size_t>(),
      .dx = sprite["dx"].template get<size_t>(),
      .dy = sprite["dy"].template get<size_t>(),
      .cols = cols,
      .rows = static_cast<size_t>(std::ceil(row_ratio))
    };
  }
}

spritesheet::spritesheet(texture2d_data texture, std::vector<sprite_data> sprites) :
  _texture(load_texture2d(std::move(texture))) {
  
  for (auto& sprite_data : sprites) {
    vec2 scale {
      (float)(sprite_data.dx*sprite_data.rows) / (float)(sprite_data.dy*sprite_data.cols),
      1.0f
    };

    vec2 linear_offset {
      (float)(sprite_data.dx) / (float)(sprite_data.x*sprite_data.cols),
      (float)(sprite_data.dy) / (float)(sprite_data.y*sprite_data.rows)
    };

    std::vector<vec2> const_offset(sprite_data.count);
    for (size_t i = 0; i < sprite_data.count; ++i) {
      size_t row = i / sprite_data.cols;
      size_t col = i % sprite_data.cols;

      vec2 frac_a{
        (sprite_data.x0*sprite_data.cols) + (col*sprite_data.dx),
        sprite_data.y0 + (row*sprite_data.dy)
      };
      vec2 frac_b{
        sprite_data.x*sprite_data.cols,
        sprite_data.y*sprite_data.rows
      };

      const_offset[i].x = frac_a.x / frac_b.x;
      const_offset[i].y = frac_a.y / frac_b.y;
    }

    _sprites.emplace_back(std::make_pair(
      std::move(sprite_data.name),
      sprite{_texture, scale, linear_offset, std::move(const_offset)}
    ));
  }
}

sprite& spritesheet::operator[](std::string_view name) {
  auto it = std::find_if(_sprites.begin(), _sprites.end(), [name](const auto& sprite) {
    return sprite.first == name;
  });
  if (it == _sprites.end()) {
    throw ntf::error{"[shogle::spritesheet] Sprite not found: {}", name};
  }
  return it->second;
}

spritesheet::spritesheet(spritesheet&& s) noexcept :
  _texture(std::move(s._texture)), _sprites(std::move(s._sprites)) {
    for (auto& [name, spr] : _sprites) {
      spr._texture = &_texture;
    }
}

spritesheet load_spritesheet(std::string_view path, tex_filter filter, tex_wrap wrap) {
  return load_spritesheet(spritesheet_data{path, filter, wrap});
}

spritesheet load_spritesheet(spritesheet_data data) {
  return spritesheet{std::move(data.texture), std::move(data.sprites)};
}

} // namespace ntf::shogle
