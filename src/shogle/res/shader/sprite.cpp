#include <shogle/res/shader/sprite.hpp>

#include <shogle/core/log.hpp>

namespace {

const char* vert_src = R"glsl(
  #version 330 core

  layout (location = 0) in vec3 att_coords;
  layout (location = 1) in vec3 att_normals;
  layout (location = 2) in vec2 att_texcoords;
  out vec2 tex_coord;

  uniform mat4 model;
  uniform mat4 view;
  uniform mat4 proj;

  uniform vec2 offset_linear;
  uniform vec2 offset_const;

  void main() {
    tex_coord.x = att_texcoords.x*offset_linear.x + offset_const.x;
    tex_coord.y = att_texcoords.y*offset_linear.y + offset_const.y;

    gl_Position = proj * view * model * vec4(att_coords, 1.0f);
  }
)glsl";

const char* frag_src = R"glsl(
  #version 330 core

  in vec2 tex_coord;
  out vec4 frag_color;

  uniform sampler2D sprite_sampler;
  uniform vec4 sprite_color;

  void main() {
    vec4 out_color = sprite_color * texture(sprite_sampler, tex_coord);

    if (out_color.a < 0.1) {
      discard;
    }

    frag_color = out_color;
  }
)glsl";

}

namespace ntf::shogle {

sprite_shader::sprite_shader() {
  try {
    shader vert {vert_src, shader_type::vertex};
    vert.compile();

    shader frag {frag_src, shader_type::fragment};
    frag.compile();

    _shader.link(vert, frag);
  } catch(...) {
    log::error("[shogle::sprite_shader] Failed to compile shader");
    throw;
  }

  _model_unif = _shader.uniform_location("model");
  _view_unif = _shader.uniform_location("view");
  _proj_unif = _shader.uniform_location("proj");

  _texture_unif = _shader.uniform_location("sprite_sampler");
  _color_unif = _shader.uniform_location("sprite_color");

  _offset_linear_unif = _shader.uniform_location("offset_linear");
  _offset_const_unif = _shader.uniform_location("offset_const");
}

} // namespace ntf::shogle
