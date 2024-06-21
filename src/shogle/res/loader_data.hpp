#pragma once

#include <shogle/render/texture.hpp>

namespace ntf::shogle {

enum class material_type {
  diffuse = 0,
  specular
};

struct vertex3dnt {
  vec3 coord;
  vec3 normal;
  vec2 tex_coord;
};

struct sprite_data {
  std::string name;
  size_t count;
  size_t x, y;
  size_t x0, y0;
  size_t dx, dy;
  size_t cols, rows;
};


class texture2d_loader {
private:
  friend class spritesheet_loader;
  texture2d_loader() = default;
  
public:
  texture2d_loader(std::string_view path);

public:
  ~texture2d_loader();
  texture2d_loader(texture2d_loader&&) noexcept;
  texture2d_loader(const texture2d_loader&) = delete;
  texture2d_loader& operator=(texture2d_loader&&) noexcept;
  texture2d_loader& operator=(const texture2d_loader&) = delete;

public:
  std::string path {};
  int width {0}, height {0}, channels {0};
  tex_format format;
  __tex2d_data pixels {};
};


class cubemap_loader {
public:
  cubemap_loader(std::string_view path);

public:
  ~cubemap_loader();
  cubemap_loader(cubemap_loader&&) noexcept;
  cubemap_loader(const cubemap_loader&) = delete;
  cubemap_loader& operator=(cubemap_loader&&) noexcept;
  cubemap_loader& operator=(const cubemap_loader&) = delete;

public:
  int width {0}, height {0}, channels {0};
  tex_format format;
  __cubemap_data pixels;
};


class model_loader {
public:
  struct mesh_data {
    std::string name;
    std::vector<vertex3dnt> vertices;
    std::vector<uint> indices;
    std::vector<std::pair<texture2d_loader, material_type>> materials;
  };

public:
  model_loader(std::string_view path);

public:
  std::vector<mesh_data> meshes;
};


class spritesheet_loader {
public:
  spritesheet_loader(std::string_view path);

public:
  texture2d_loader texture;
  std::vector<sprite_data> sprites;
};

} // namespace ntf::shogle
