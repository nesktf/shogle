#version 330 core

layout (location = 0) in vec4 tex_coord_in;

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;
uniform vec4 sprite_offset;

void main() {
  tex_coord.x = tex_coord_in.z*sprite_offset.x + sprite_offset.z;
  tex_coord.y = tex_coord_in.w*sprite_offset.y + sprite_offset.w;

  gl_Position = proj * view * model * vec4(tex_coord_in.xy, 0.0f, 1.0f);
}
