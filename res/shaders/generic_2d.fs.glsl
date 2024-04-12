#version 330 core

in vec2 tex_coord;

out vec4 frag_color;

uniform sampler2D texture_sampler;
uniform vec4 texture_color;

void main() {
  vec4 out_color = texture_color * texture(texture_sampler, tex_coord);

  if (out_color.a < 0.1) {
    discard;
  }

  frag_color = out_color;
}
