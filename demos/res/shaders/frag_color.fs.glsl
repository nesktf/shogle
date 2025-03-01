#version 460 core

out vec4 frag_color;

uniform vec4 color;

void main() {
  vec4 out_color = color;

  if (out_color.a < 0.1) {
    discard;
  }

  frag_color = out_color;
}
