#version 460 core

layout (location = 0) in vec3 att_coords;
layout (location = 1) in vec3 att_normals;
layout (location = 2) in vec2 att_texcoords;

out vec2 tex_coord;

uniform mat4 proj;
// uniform mat4 view;

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
  gl_Position = proj*vec4(coords, 0.f, 1.0f);

  tex_coord.x = att_texcoords.x*glyphs[gl_InstanceID].uv_scale.x
              + glyphs[gl_InstanceID].uv_offset.x;
  tex_coord.y = att_texcoords.y*glyphs[gl_InstanceID].uv_scale.y
              + glyphs[gl_InstanceID].uv_offset.y;
}
