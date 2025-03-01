#version 460 core

in vec2 tex_coord;
out vec4 frag_color;

uniform vec4 color;
uniform sampler2D sampler0;

void main()  {
  vec4 out_color = color * texture(sampler0, tex_coord);

  if (out_color.a < 0.1) {
    discard;
  }

  frag_color = out_color;
}


