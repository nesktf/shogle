#include <shogle/res/shader/font.hpp>

#include <shogle/core/log.hpp>

static const char* vert_src = R"glsl(
  #version 330 core

  layout(location = 0) in vec4 att_vertex;
  out vec2 tex_coord;

  uniform mat4 proj;

  void main() {
    gl_Position = proj * vec4(att_vertex.xy, 0.0f, 1.0f);
    tex_coord = att_vertex.zw;
  }
)glsl";

static const char* frag_src = R"glsl(
  #version 330 core

  in vec2 tex_coord;
  out vec4 frag_color;

  uniform sampler2D tex;
  uniform vec4 text_color;

  void main() {
    vec4 sampled = vec4(1.0f, 1.0f, 1.0f, texture(tex, tex_coord).r);
    frag_color = text_color * sampled;
  }
)glsl";

namespace ntf::shogle {

font_shader::font_shader() {
  try {
    shader vert {vert_src, shader_type::vertex};
    vert.compile();

    shader frag {frag_src, shader_type::fragment};
    frag.compile();

    _shader.link(vert, frag);
  } catch(...) {
    log::error("[shogle::font_shader] Failed to compile shader");
    throw;
  }

  _proj_unif = _shader.uniform_location("proj");
  _color_unif = _shader.uniform_location("text_color");
  _sampler_unif = _shader.uniform_location("tex");
}

} // namespace ntf::shogle
