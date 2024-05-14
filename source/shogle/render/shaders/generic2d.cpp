#include <shogle/render/shaders/generic2d.hpp>

#include <shogle/core/log.hpp>

namespace {

const char* vert_src = R"glsl(
  #version 330 core

  layout (location = 0) in vec2 att_coords;
  layout (location = 1) in vec2 att_texcoords;
  out vec2 tex_coord;

  uniform mat4 model;
  uniform mat4 view;
  uniform mat4 proj;

  uniform vec2 offset_linear;
  uniform vec2 offset_const;

  void main() {
    tex_coord.x = att_texcoords.x*offset_linear.x + offset_const.x;
    tex_coord.y = att_texcoords.y*offset_linear.y + offset_const.y;

    gl_Position = proj * view * model * vec4(att_coords, 0.0f, 1.0f);
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

namespace ntf::shogle::shaders {

generic2d::generic2d() {
  try {
    gl::shader vert {std::string{vert_src}, gl::shader::type::vertex};
    vert.compile();

    gl::shader frag {std::string{frag_src}, gl::shader::type::fragment};
    frag.compile();

    attach_shaders(std::move(vert), std::move(frag));
  } catch(...) {
    Log::error("[shaders::generic2d] Failed to build shader");
    throw;
  }

  _model_unif = uniform_location("model");
  _view_unif = uniform_location("view");
  _proj_unif = uniform_location("proj");

  _texture_unif = uniform_location("sprite_sampler");
  _color_unif = uniform_location("sprite_color");

  _offset_linear_unif = uniform_location("offset_linear");
  _offset_const_unif = uniform_location("offset_const");
}

} // namespace ntf::shogle::shaders
