#pragma once

#include <shogle/render/res/texture.hpp>
#include <shogle/fs/res/model.hpp>

#include <vector>

namespace ntf::render {

class Model {
public:
  using data_t = fs::model_data;

  class Material : public Texture {
  public:
    using data_t = fs::material_data;

  public:
    Material(const Material::data_t* data);

  public:
    void bind_uniform(const Shader* shader, size_t tex_num, size_t tex_ind) const;

  public:
    fs::material_type type(void) const { return _type; }

  private:
    fs::material_type _type {fs::material_type::diffuse};
    std::string _uniform_basename {"material.diffuse"};
  };

  class Mesh {
  public: // Resources can't be copied
    Mesh(const fs::mesh_data& mesh);
    ~Mesh();

    Mesh(Mesh&&) noexcept;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(Mesh&&) noexcept;
    Mesh& operator=(const Mesh&) = delete;

  public:
    GLuint id(void) const { return _vao; }
    size_t ind(void) const { return _indices; }

  public:
    std::vector<Material> materials;

  private:
    GLuint _vao, _vbo, _ebo;
    size_t _indices;
  };

public: // Resource wrappers also can't be copied
  Model(const Model::data_t* data);
  ~Model() = default;

  Model(Model&&) = default;
  Model(const Model&) = delete;
  Model& operator=(Model&&) = default;
  Model& operator=(const Model&) = delete;

public:
  std::vector<Mesh> meshes;
};

} // namespace ntf::render
