#version 330 core

in vec2 tex_coord;

out vec4 frag_color;

uniform sampler2D sprite_sampler;
uniform vec4 sprite_color;

void main() {
  vec4 out_color = sprite_color * texture(sprite_sampler, tex_coord);

  if (out_color.a < 0.1) {
    discard;
  }

  frag_color = out_color;
}
