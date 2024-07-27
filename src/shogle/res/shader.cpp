#include <shogle/res/shader.hpp>

#include <shogle/core/log.hpp>

namespace ntf::shogle {

static const char* sprite_vert = R"glsl(
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

static const char* sprite_frag = R"glsl(
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

void sprite_shader::compile() {
  try {
    _shader = load_shader_program(sprite_vert, sprite_frag);
  } catch(...) {
    log::error("[shogle::sprite_shader] Failed to compile shader");
    throw;
  }

  _model_unif = _shader.uniform_location("model");
  _view_unif = _shader.uniform_location("view");
  _proj_unif = _shader.uniform_location("proj");

  _sampler_unif = _shader.uniform_location("sprite_sampler");
  _color_unif = _shader.uniform_location("sprite_color");

  _offset_linear_unif = _shader.uniform_location("offset_linear");
  _offset_const_unif = _shader.uniform_location("offset_const");
}


static const char* framebuffer_vert = R"glsl(
  #version 330 core

  layout (location = 0) in vec3 att_coords;
  layout (location = 1) in vec3 att_normals;
  layout (location = 2) in vec2 att_texcoords;
  out vec2 tex_coord;

  uniform mat4 model;
  uniform mat4 view;
  uniform mat4 proj;

  void main() {
    gl_Position = proj * view * model * vec4(att_coords, 1.0f);
    tex_coord = vec2(att_texcoords.x, 1-att_texcoords.y);
  }
)glsl";

static const char* framebuffer_frag = R"glsl(
  #version 330 core

  in vec2 tex_coord;
  out vec4 frag_color;

  uniform sampler2D fb_sampler;

  void main() {
    frag_color = texture(fb_sampler, tex_coord);
  }
)glsl";

void framebuffer_shader::compile() {
  try {
    _shader = load_shader_program(framebuffer_vert, framebuffer_frag);
  } catch(...) {
    log::error("[shogle::framebuffer_shader] Failed to compile shader");
    throw;
  }

  _model_unif = _shader.uniform_location("model");
  _view_unif = _shader.uniform_location("view");
  _proj_unif = _shader.uniform_location("proj");

  _sampler_unif = _shader.uniform_location("fb_sampler");
}


static const char* font_vert = R"glsl(
  #version 330 core

  layout(location = 0) in vec4 att_vertex;
  out vec2 tex_coord;

  uniform mat4 proj;

  void main() {
    gl_Position = proj * vec4(att_vertex.xy, 0.0f, 1.0f);
    tex_coord = att_vertex.zw;
  }
)glsl";

static const char* font_frag = R"glsl(
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

void font_shader::compile() {
  try {
    _shader = load_shader_program(font_vert, font_frag);
  } catch(...) {
    log::error("[shogle::font_shader] Failed to compile shader");
    throw;
  }

  _proj_unif = _shader.uniform_location("proj");
  _color_unif = _shader.uniform_location("text_color");
  _sampler_unif = _shader.uniform_location("tex");
}


static const char* model_vert = R"glsl(
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

static const char* model_frag = R"glsl(
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

void model_shader::compile() {
  try {
    _shader = load_shader_program(model_vert, model_frag);
  } catch(...) {
    log::error("[shogle::model_shader] Failed to compile shader");
    throw;
  }

  _model_unif = _shader.uniform_location("model");
  _view_unif = _shader.uniform_location("view");
  _proj_unif = _shader.uniform_location("proj");

  _diffuse_sampler_unif = _shader.uniform_location("material.diffuse");
  _specular_sampler_unif = _shader.uniform_location("material.specular");
  _shiny_unif = _shader.uniform_location("material.shiny");
}


static const char* skybox_vert = R"glsl(
  #version 330 core

  layout (location = 0) in vec3 att_coords;
  out vec3 texcoord;

  uniform mat4 proj;
  uniform mat4 view;

  void main() {
    vec4 pos = proj * view * vec4(att_coords, 1.0f);
    gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
    texcoord = vec3(att_coords.x, att_coords.y, -att_coords.z);
  }

)glsl";

static const char* skybox_frag = R"glsl(
  #version 330 core

  in vec3 texcoord;
  out vec4 frag_color;

  uniform samplerCube skybox;

  void main() {
    frag_color = texture(skybox, texcoord);
  }
)glsl";

void skybox_shader::compile() {
  try {
    _shader = load_shader_program(skybox_vert, skybox_frag);
  } catch(...) {
    log::error("[shogle::skybox_shader] Failed to compile shader");
    throw;
  }

  _view_unif = _shader.uniform_location("view");
  _proj_unif = _shader.uniform_location("proj");

  _sampler_unif = _shader.uniform_location("skybox");
}

} // namespace ntf::shogle
