#pragma once

#include <string_view>

namespace ntf {

constexpr inline std::string_view shad_vert_instanced_font = R"glsl(
#version 460 core

layout (location = 0) in vec3 att_coords;
layout (location = 1) in vec3 att_normals;
layout (location = 2) in vec2 att_texcoords;

out vec3 vert_normals;
out vec2 tex_coord;

uniform mat4 u_text_transform;

struct font_shader_glyph {
  vec2 transf_scale;
  vec2 transf_offset;
  vec2 uv_scale;
  vec2 uv_offset;
};

layout(std430, binding = 1) buffer glyph_offsets
{
  font_shader_glyph glyphs[];
};

void main() {
  vec2 coords;
  coords.x = glyphs[gl_InstanceID].transf_scale.x*att_coords.x
           + glyphs[gl_InstanceID].transf_offset.x;
  coords.y = glyphs[gl_InstanceID].transf_scale.y*att_coords.y
           + glyphs[gl_InstanceID].transf_offset.y;
  gl_Position = u_text_transform*vec4(coords, 0.f, 1.0f);

  tex_coord.x = att_texcoords.x*glyphs[gl_InstanceID].uv_scale.x
              + glyphs[gl_InstanceID].uv_offset.x;
  tex_coord.y = att_texcoords.y*glyphs[gl_InstanceID].uv_scale.y
              + glyphs[gl_InstanceID].uv_offset.y;
  vert_normals = att_normals;
}
)glsl";

constexpr inline std::string_view shad_frag_font_sdf = R"glsl(
#version 460 core

in vec3 vert_normals;
in vec2 tex_coord;

out vec4 frag_color;

uniform sampler2D u_atlas_sampler;

uniform vec3 u_text_color;
uniform float u_text_width;
uniform float u_text_edge;

uniform vec3 u_text_out_color;
uniform vec2 u_text_out_offset;
uniform float u_text_out_width;
uniform float u_text_out_edge;

void main() {
  const float text_sdf = 1.f-texture(u_atlas_sampler, tex_coord).r;
  const float outline_sdf = 1.f-texture(u_atlas_sampler, tex_coord+u_text_out_offset).r;

  const float text_alpha = 1.f-smoothstep(u_text_width, u_text_width+u_text_edge,
                                          text_sdf);
  const float outline_alpha = 1.f-smoothstep(u_text_out_width, u_text_out_width+u_text_out_edge,
                                             outline_sdf);
  
  const float total_alpha = text_alpha+(1.f-text_alpha)*outline_alpha;
  const vec3 total_color = mix(u_text_out_color, u_text_color, text_alpha/total_alpha);

  frag_color = vec4(total_color, total_alpha);
}
)glsl";

constexpr inline std::string_view shad_frag_font_normal = R"glsl(
#version 460 core

in vec3 vert_normals;
in vec2 tex_coord;

out vec4 frag_color;

uniform sampler2D u_atlas_sampler;

uniform vec3 u_text_color;

void main() {
  const float text_alpha = texture(u_atlas_sampler, tex_coord).r;
  frag_color = vec4(u_text_color, text_alpha);
}

)glsl";

} // namespace ntf
