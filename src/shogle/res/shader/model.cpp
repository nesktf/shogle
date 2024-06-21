#include <shogle/res/shader/model.hpp>

#include <shogle/core/log.hpp>

namespace {

const char* vert_src = R"glsl(
  #version 330 core

  layout (location = 0) in vec3 att_coords;
  layout (location = 1) in vec3 att_normals;
  layout (location = 2) in vec2 att_texcoords;

  uniform mat4 model;
  uniform mat4 view;
  uniform mat4 proj;

  out vec3 normal;
  out vec2 texcoord;

  void main() {
    gl_Position = proj * view * model * vec4(att_coords, 1.0f);
    normal = att_normals;
    texcoord = att_texcoords;
  }
)glsl";

const char* frag_src = R"glsl(
  #version 330 core

  in vec3 normal;
  in vec2 texcoord;
  out vec4 frag_color;

  struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shiny;
  };
  uniform Material material;

  void main() {
    vec4 out_color = texture(material.diffuse, texcoord);

    if (out_color.a < 0.1)
      discard;

    frag_color = out_color;
  }
)glsl";

}

namespace ntf::shogle {

model_shader::model_shader() {
  try {
    shader vert {vert_src, shader_type::vertex};
    vert.compile();

    shader frag {frag_src, shader_type::fragment};
    frag.compile();

    _shader.link(vert, frag);
  } catch(...) {
    log::error("[shogle::model_shader] Failed to compile shader");
    throw;
  }

  _model_unif = _shader.uniform_location("model");
  _view_unif = _shader.uniform_location("view");
  _proj_unif = _shader.uniform_location("proj");

  _diffuse_unif = _shader.uniform_location("material.diffuse");
  _specular_unif = _shader.uniform_location("material.specular");
  _shiny_unif = _shader.uniform_location("material.shiny");
}

} // namespace ntf::shogle
