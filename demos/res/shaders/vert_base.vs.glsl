#version 460 core

layout (location = 0) in vec3 att_coords;
layout (location = 1) in vec3 att_normals;
layout (location = 2) in vec2 att_texcoords;

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;

void main() {
  gl_Position = proj*view*model*vec4(att_coords, 1.0f);
  tex_coord = att_texcoords;
}
