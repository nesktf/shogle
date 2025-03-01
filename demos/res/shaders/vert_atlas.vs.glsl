#version 460 core

layout (location = 0) in vec3 att_coords;
layout (location = 1) in vec3 att_normals;
layout (location = 2) in vec2 att_texcoords;
out vec2 tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec4 offset;

void main() {
  tex_coord.x = att_texcoords.x*offset.x + offset.z;
  tex_coord.y = att_texcoords.y*offset.y + offset.w;

  gl_Position = proj * view * model * vec4(att_coords, 1.0f);
}
