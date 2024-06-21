#include <shogle/res/shader/skybox.hpp>

#include <shogle/core/log.hpp>

namespace {

const char* vert_src = R"glsl(
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

const char* frag_src = R"glsl(
  #version 330 core

  in vec3 texcoord;
  out vec4 frag_color;

  uniform samplerCube skybox;

  void main() {
    frag_color = texture(skybox, texcoord);
  }
)glsl";

}

namespace ntf::shogle {

skybox_shader::skybox_shader() {
  try {
    shader vert {vert_src, shader_type::vertex};
    vert.compile();

    shader frag {frag_src, shader_type::fragment};
    frag.compile();

    _shader.link(vert, frag);
  } catch(...) {
    log::error("[shogle::skybox_shader] Failed to compile shader");
    throw;
  }

  _view_unif = _shader.uniform_location("view");
  _proj_unif = _shader.uniform_location("proj");

  _cubemap_unif = _shader.uniform_location("skybox");
}

} // namespace ntf::shogle
