#include <shogle/scene/sprite.hpp>

#include <shogle/core/engine.hpp>
#include <shogle/core/log.hpp>

namespace ntf {

SpriteImpl::SpriteImpl(const Texture* tex, const Shader* sha) :
  SpriteRenderer(tex, sha),
  _sprite(SpriteData{
      .count = 1,
      .x = tex->width(),
      .y = tex->height(),
      .x0 = 0,
      .y0 = 0,
      .dx = tex->width(),
      .dy = tex->height(),
      .cols = 1,
      .rows = 1
    }),
  _index(0),
  cam(&Shogle::instance().cam2D_default) { 
    alt_draw = tex->inverted;
    scale = corrected_scale(); 
}

SpriteImpl::SpriteImpl(const Spritesheet* sheet, std::string name, const Shader* sha) :
  SpriteRenderer(static_cast<const Texture*>(sheet), sha),
  _sprite(sheet->sprites.at(name)),
  cam(&Shogle::instance().cam2D_default) {
    _offset.x = static_cast<float>(_sprite.dx)/static_cast<float>(_sprite.x*_sprite.cols);
    _offset.y = static_cast<float>(_sprite.dy)/static_cast<float>(_sprite.y*_sprite.rows);
    set_index(0); 
    alt_draw = sheet->inverted;
    scale = corrected_scale();
}

void SpriteImpl::update(float) {
  _model_mat = _gen_model();
  _shader->use();
  _shader_update();
}

void SpriteImpl::_shader_update(void) {
  _shader->unif_mat4("proj", cam->proj_mat());
  _shader->unif_mat4("view", use_screen_space ? mat4{1.0f} : cam->view_mat());
  _shader->unif_mat4("model", _model_mat);
  _shader->unif_vec4("texture_color", color);
  _shader->unif_vec4("texture_offset", _offset);
  _shader->unif_int("texture_sampler", 0);
}

mat4 SpriteImpl::_gen_model(void) {
  mat4 mat{1.0f};

  mat = glm::translate(mat, vec3{pos, static_cast<float>(layer)});
  mat = glm::rotate(mat, rot, vec3{0.0f, 0.0f, 1.0f});
  mat = glm::scale(mat, vec3{scale, 1.0f});

  return mat;
}

void SpriteImpl::set_index(size_t i) {
  i = i % _sprite.count; // don't overflow
  size_t row = i / _sprite.cols;
  size_t col = i % _sprite.cols;

  vec2 frac_a {
    static_cast<float>(_sprite.x0 + (col*_sprite.dx)),
    static_cast<float>(_sprite.y0 + (row*_sprite.dy))
  };

  vec2 frac_b {
    static_cast<float>(_sprite.x*_sprite.cols),
    static_cast<float>(_sprite.y*_sprite.rows)
  };

  _offset.z = frac_a.x / frac_b.x;
  _offset.w = frac_a.y / frac_b.y;

  _index = i;
}

} // namespace ntf
