#version 460 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D sampler0;
uniform vec4 text_color;

const float w = 0.5f;
const float edge = 0.05f;

const float border_w = 0.62f;
const float border_edge = 0.05f;
const vec3 outline = vec3(0.f, 0.f, 0.f);

void main() {
  float sdf = 1.f-texture(sampler0, tex_coord).r;
  float alpha = 1.f-smoothstep(w, w+edge, sdf);
  float out_alpha = 1.f-smoothstep(border_w, border_w+border_edge, sdf);

  float all = alpha + (1.f - alpha) * out_alpha;
  vec3 all_color = mix(outline, text_color.rgb, alpha/all);

  frag_color = vec4(all_color, all);
}
