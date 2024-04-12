#version 330 core

layout (location = 0) in vec4 tex_coord_in;

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 proj;
uniform vec4 texture_offset;

void main() {
  tex_coord.x = tex_coord_in.z*texture_offset.x + texture_offset.z;
  tex_coord.y = tex_coord_in.w*texture_offset.y + texture_offset.w;

  gl_Position = proj * model * vec4(tex_coord_in.xy, 0.0f, 1.0f);
}
