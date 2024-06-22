#include <shogle/res/spritesheet.hpp>

#include <shogle/res/util.hpp>

#include <nlohmann/json.hpp>

#include <fstream>

namespace ntf::shogle {

spritesheet::spritesheet(spritesheet_loader loader) :
  _texture(
    vec2sz{loader.texture.width, loader.texture.height},
    loader.texture.format,
    tex_filter::nearest,
    tex_wrap::repeat,
    std::move(loader.texture.pixels)
  ) {
  
  for (auto& sprite_data : loader.sprites) {
    sprite sprite{_texture, sprite_data.count};

    sprite._corrected_scale.x =
      (float)(sprite_data.dx*sprite_data.rows) / (float)(sprite_data.dy*sprite_data.cols);
    sprite._corrected_scale.y = 1.0f;

    sprite._linear_offset.x = 
      (float)(sprite_data.dx) / (float)(sprite_data.x*sprite_data.cols);
    sprite._linear_offset.y =
      (float)(sprite_data.dy) / (float)(sprite_data.y*sprite_data.rows);

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

      sprite._const_offset[i].x = frac_a.x / frac_b.x;
      sprite._const_offset[i].y = frac_a.y / frac_b.y;
    }

    _sprites.emplace(std::make_pair(std::move(sprite_data.name), std::move(sprite)));
  }

  // "sprite" for the whole spritesheet
  sprite __sheet{_texture, 1};
  __sheet._corrected_scale.x =
    (float)(loader.texture.width) / (float)(loader.texture.height);
  __sheet._corrected_scale.y = 1.0f;
  __sheet._linear_offset = vec2{1.0f};
  __sheet._const_offset[0] = vec2{0.0f};
  _sprites.emplace(std::make_pair(std::string{"__sheet"}, std::move(__sheet)));
}

} // namespace ntf::shogle
