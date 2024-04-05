#version 330 core

in vec2 tex_coord;
out vec4 FragColor;

uniform sampler2D sprite_texture;
uniform vec4 sprite_color;

void main() {
  // FragColor = vec4(texture(material.texture_diffuse1,Texture).rgb, 1.0f);
  vec4 out_color = sprite_color * texture(sprite_texture, tex_coord);
  if (out_color.a < 0.1)
    discard;
  FragColor = out_color;
  // FragColor = vec4(1.0f);
}
