#version 460 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D sampler0;
uniform vec4 text_color;
uniform float time;

void main() {
  vec2 coord = tex_coord + time*0.05*vec2(1.f, 0.f);
  vec4 sampled = vec4(1.0f, 1.0f, 1.0f, texture(sampler0, coord).r);
  frag_color = text_color * sampled;
}
