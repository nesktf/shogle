#include <shogle/render/sprite.hpp>

namespace ntf::render {

sprite::sprite(std::string path) :
  sprite(loader_t{path}) {}

sprite::sprite(loader_t loader) :
  _tex(std::move(loader)),
  _unique(true),
  _linear_off(vec2{1.0f}),
  _const_off{vec2{0.0f}} {

  _aspect = static_cast<float>(_tex.width)/static_cast<float>(_tex.height);
}

sprite::sprite(gl::texture tex, size_t w, size_t h) :
  _tex(tex), 
  _unique(false),
  _linear_off(vec2{1.0f}),
  _const_off{vec2{0.0f}} {

  _aspect = static_cast<float>(w)/static_cast<float>(h);
}

sprite::sprite(gl::texture tex, spritedata_t data) :
  _tex(tex),
  _unique(false),
  _const_off(data.count) {

  _aspect =
    static_cast<float>(data.dx*data.rows) / static_cast<float>(data.dy*data.cols);

  // sprite texture offset pre-calculations
  _linear_off.x = 
    static_cast<float>(data.dx)/static_cast<float>(data.x*data.cols);
  _linear_off.y = 
    static_cast<float>(data.dy)/static_cast<float>(data.y*data.rows);

  for (size_t i = 0; i < data.count; ++i) {
    size_t row = i / data.cols;
    size_t col = i % data.cols;

    vec2 frac_a {
      data.x0 + (col*data.dx),
      data.y0 + (row*data.dy)
    };

    vec2 frac_b {
      data.x*data.cols,
      data.y*data.rows
    };

    _const_off[i].x = frac_a.x / frac_b.x;
    _const_off[i].y = frac_a.y / frac_b.y;
  }
}

sprite& sprite::operator=(sprite&& s) noexcept {
  if (_unique) {
    gl::destroy_texture(_tex);
  }

  _tex = std::move(s._tex);
  _unique = std::move(s._unique);
  _linear_off = std::move(s._linear_off);
  _const_off = std::move(s._const_off);
  _aspect = std::move(s._aspect);

  s._tex.id = 0; // avoid destroying moved gl handle

  return *this;
}

sprite::~sprite() {
  if (_unique) {
    gl::destroy_texture(_tex);
  }
}


spritesheet::spritesheet(std::string path) :
  spritesheet(loader_t{std::move(path)}) {}

spritesheet::spritesheet(loader_t loader) :
  _tex(std::move(loader.tex)) {
  for (const auto& sprite_entry : loader.sprites) {
    auto& name = sprite_entry.first;
    auto& spr_data = sprite_entry.second;

    _sprites.emplace(std::make_pair(name, sprite{_tex, spr_data}));
  }
}

spritesheet& spritesheet::operator=(spritesheet&& s) noexcept {
  gl::destroy_texture(_tex);

  _tex = std::move(s._tex);
  _sprites = std::move(s._sprites);

  s._tex.id = 0; // avoid destroying moved gl handle

  return *this;
}

spritesheet::~spritesheet() {
  gl::destroy_texture(_tex);
}


void draw_sprite(sprite& sprite, shader& shader, size_t index, bool inverted) {
  shader.use();
  shader.set_uniform("sprite_offset", sprite.uniform_offset(index));
  shader.set_uniform("sprite_sampler", 0);

  gl::texture_bind(sprite._tex, 0);
  gl::draw_quad_2d(inverted);
}

} // namespace ntf::render
