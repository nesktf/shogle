#version 330 core

layout (location = 0) in vec4 sprite_coords;

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 proj;

void main() {
  tex_coord = sprite_coords.zw;
  gl_Position = proj * model * vec4(sprite_coords.xy, 0.0f, 1.0f);
}
