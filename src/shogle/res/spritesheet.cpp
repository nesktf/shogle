#include <shogle/res/spritesheet.hpp>

#include <shogle/res/util.hpp>

#include <shogle/core/error.hpp>
#include <shogle/core/log.hpp>

#include <nlohmann/json.hpp>

#include <fstream>

using json = nlohmann::json;

namespace ntf::shogle {

spritesheet_data::spritesheet_data(std::string_view path_) {
  std::ifstream f{path_.data()};
  json data = json::parse(f);

  auto texture_path = file_dir(path_.data())+"/"+data["file"].template get<std::string>();
  texture = texture2d_data{texture_path};

  auto sprite_content = data["content"];
  for (size_t i = 0; i < sprite_content.size(); ++i) {
    auto& sprite = sprite_content[i];

    std::string name = sprite["name"].get<std::string>();

    size_t x = static_cast<size_t>(texture.width);
    size_t y = static_cast<size_t>(texture.height);

    size_t x0 = sprite["x0"].get<size_t>();
    size_t y0 = sprite["y0"].get<size_t>();
    size_t dx = sprite["dx"].get<size_t>();
    size_t dy = sprite["dy"].get<size_t>();

    uint delay = sprite["delay"].get<uint>();

    size_t count = sprite["count"].get<size_t>();
    size_t cols = sprite["cols"].get<size_t>();
    float row_ratio = static_cast<float>(count)/static_cast<float>(cols);
    size_t rows = static_cast<size_t>(std::ceil(row_ratio));

    vec2 scale { (float)(dx*rows) / (float)(dy*cols), 1.0f };

    vec2 linear_offset = { (float)dx / (float)(x*cols), (float)dy / (float)(y*rows) };

    std::vector<vec2> const_offset(count);
    for (size_t j = 0; j < count; ++j) {
      size_t row = j / cols;
      size_t col = j % cols;

      vec2 frac_a{ (x0*cols) + (col*dx), y0 + (row*dy) };
      vec2 frac_b{ x*cols, y*rows };
      const_offset[j].x = frac_a.x/frac_b.x;
      const_offset[j].y = frac_a.y/frac_b.y;
    }

    sprites.emplace(std::make_pair(std::move(name), sprite_data{
      .delay = delay,
      .scale = scale,
      .linear_offset = linear_offset,
      .const_offset = std::move(const_offset)
    }));
  }
}

sprite::sprite(const texture2d* tex, const std::vector<vec2>* coff, vec2 loff, vec2 scale, uint delay) :
  _tex(tex), _const_offset(coff), _linear_offset(loff), _offset(offset_at(0)), _scale(scale), _delay(delay) {}

sprite spritesheet::at(std::string_view name) const {
  const auto& data = _sprites.at(name.data());
  return sprite{&_texture, &data.const_offset, data.linear_offset, data.scale, data.delay};
}

sprite_animator::sprite_animator(uint delay_, sprite_animation animation_) :
  delay(delay_), animation(animation_) {}

void sprite_animator::tick(sprite& sprite) {
  if (++_index_time >= delay) {
    size_t c = sprite.count();
    bool animate_forwards = (animation & sprite_animation::forward_anim) != sprite_animation::none;
    
    // Do nothing if the animation can't repeat and the index wraps around
    bool repeat = (animation & sprite_animation::repeat) != sprite_animation::none;
    if (!repeat && (animate_forwards ? _index == c-1 : _index == 0)) {
      return;
    }

    if (animate_forwards) {
      sprite.set_index(++_index % c);
    } else {
      _index = _index-1 < 0 ? c-1 : _index-1;
      sprite.set_index(_index);
    }

    _index_time = 0;
  }
}

void sprite_animator::set_index(sprite& sprite, uint index) {
  _index = index % sprite.count();
  sprite.set_index(_index);
}

} // namespace ntf::shogle
