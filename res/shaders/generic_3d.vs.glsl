#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexture;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 view_pos;
// uniform vec2 tex_offset;

out vec3 Normal;
out vec2 Texture;

void main()
{
  gl_Position = proj * view * model * vec4(aPos, 1.0f);
  Normal = aNormal;
  Texture = aTexture;
  // Texture = vec2(aTexture.x+tex_offset.x, aTexture.y+tex_offset.y);
}
