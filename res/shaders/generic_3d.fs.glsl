#version 330 core

in vec3 Normal;
in vec2 Texture;

out vec4 FragColor;

// uniform vec3 objColor;
// uniform vec3 lightCol;
struct Material {
  sampler2D diffuse0;
  sampler2D specular0;
  float col_shiny;
};
uniform Material material;
// uniform sampler2D fb_tex;

// uniform vec3 lightCol;

void main()
{
  // FragColor = vec4(texture(material.texture_diffuse1,Texture).rgb, 1.0f);
  vec4 texColor = texture(material.diffuse1,Texture);
  if (texColor.a < 0.1)
    discard;
  FragColor = texColor;
  // FragColor = vec4(1.0f);
}
