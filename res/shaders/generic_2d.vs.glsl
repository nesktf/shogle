#version 330 core

layout (location = 0) in vec4 sprite_coords;

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 proj;

uniform vec4 sprite_offset;

void main() {
  tex_coord.x = (sprite_coords.z + sprite_offset.x)*sprite_offset.z;
  tex_coord.y = (sprite_coords.w + sprite_offset.y)*sprite_offset.w;

  gl_Position = proj * model * vec4(sprite_coords.xy, 0.0f, 1.0f);
}
