#include <shogle/render/sprite.hpp>

#include <shogle/render/shader.hpp>

namespace ntf::render {

spritesheet::spritesheet(loader_t loader) {
  _tex = make_uptr<renderer::texture>(std::move(loader.tex));
  for (const auto& sprite_entry : loader.sprites) {
    auto& name = sprite_entry.first;
    auto& spr_data = sprite_entry.second;

    _sprites.emplace(std::make_pair(name, sprite{_tex.get(), spr_data}));
  }
}

sprite::sprite(renderer::texture* tex, data_t data) :
  _tex(tex), _uniform_offset_const(data.count) {

  _aspect =
    static_cast<float>(data.dx*data.rows) / static_cast<float>(data.dy*data.cols);

  // sprite texture offset pre-calculations
  _uniform_offset_linear.x = 
    static_cast<float>(data.dx)/static_cast<float>(data.x*data.cols);
  _uniform_offset_linear.y = 
    static_cast<float>(data.dy)/static_cast<float>(data.y*data.rows);

  for (size_t i = 0; i < data.count; ++i) {
    size_t row = i / data.cols;
    size_t col = i % data.cols;

    vec2 frac_a {
      static_cast<float>(data.x0 + (col*data.dx)),
      static_cast<float>(data.y0 + (row*data.dy))
    };

    vec2 frac_b {
      static_cast<float>(data.x*data.cols),
      static_cast<float>(data.y*data.rows)
    };

    _uniform_offset_const[i].x = frac_a.x / frac_b.x;
    _uniform_offset_const[i].y = frac_a.y / frac_b.y;
  }

}

void sprite::draw(shader& shader, size_t index) const {
  vec4 _offset {_uniform_offset_linear, _uniform_offset_const[index % count()]};

  shader.use();
  shader.set_uniform("sprite_offset", _offset);
  shader.set_uniform("sprite_sampler", 0);

  renderer::draw_quad(_tex);
}


} // namespace ntf::render
