#include <shogle/res/global.hpp>

#include <shogle/core/log.hpp>

namespace ntf::res {

// default sprite vertex shader
const char* def_sprite_vert = R"glsl(
  #version 330 core

  layout (location = 0) in vec4 coords_att; // xy vert, zw tex
  out vec2 tex_coord;

  uniform mat4 model;
  uniform mat4 proj;
  uniform mat4 view;
  uniform vec4 sprite_offset;

  void main() {
    tex_coord.x = coords_att.z*sprite_offset.x + sprite_offset.z;
    tex_coord.y = coords_att.w*sprite_offset.y + sprite_offset.w;

    gl_Position = proj * view * model * vec4(coords_att.xy, 0.0f, 1.0f);
  }
)glsl";

// default sprite fragment shader
const char* def_sprite_frag = R"glsl(
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

// default model vertex shader
const char* def_model_vert = R"glsl(
  #version 330 core

  layout (location = 0) in vec3 vert_coord_att;
  layout (location = 1) in vec3 vert_normal_att;
  layout (location = 2) in vec2 tex_coord_att;

  uniform mat4 model;
  uniform mat4 view;
  uniform mat4 proj;
  uniform vec3 view_pos;

  out vec3 vert_normal;
  out vec2 tex_coord;

  void main() {
    gl_Position = proj * view * model * vec4(vert_coord_att, 1.0f);
    vert_normal = vert_normal_att;
    tex_coord = tex_coord_att;
  }
)glsl";

// default model fragment shader
const char* def_model_frag = R"glsl(
  #version 330 core

  in vec3 vert_normal;
  in vec2 tex_coord;
  out vec4 frag_color;

  struct Material {
    sampler2D diffuse0;
    sampler2D specular0;
    float col_shiny;
  };
  uniform Material material;

  void main() {
    vec4 out_color = texture(material.diffuse0, tex_coord);

    if (out_color.a < 0.1)
      discard;

    frag_color = out_color;
  }
)glsl";

// default skybox vertex shader
const char* def_skybox_vert = R"glsl(
  #version 330 core

  layout (location = 0) in vec3 vert_coord_att;
  out vec3 tex_coord;

  uniform mat4 proj;
  uniform mat4 view;

  void main() {
    vec4 pos = proj * view * vec4(vert_coord_att, 1.0f);
    gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
    tex_coord = vec3(vert_coord_att.x, vert_coord_att.y, -vert_coord_att.z);
  }

)glsl";

// default skybox fragment shader
const char* def_skybox_frag = R"glsl(
  #version 330 core

  in vec3 tex_coord;
  out vec4 frag_color;

  uniform samplerCube skybox;

  void main() {
    frag_color = texture(skybox, tex_coord);
  }
)glsl";

// init default things
uptr<render::shader> def_sprite_sh {};
uptr<render::shader> def_model_sh {};
uptr<render::shader> def_skybox_sh {};

camera2d def_cam2d{};
camera3d def_cam3d{};

void init_def() {
  def_sprite_sh = make_uptr<render::shader>(def_sprite_vert, def_sprite_frag);
  def_model_sh = make_uptr<render::shader>(def_model_vert, def_model_frag);
  def_skybox_sh = make_uptr<render::shader>(def_skybox_vert, def_skybox_frag);
}

void destroy_def() {
  def_skybox_sh.reset();
  def_model_sh.reset();
  def_sprite_sh.reset();
}

} // namespace ntf::res