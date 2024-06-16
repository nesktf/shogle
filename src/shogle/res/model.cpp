#include <shogle/res/model.hpp>

#include <shogle/res/util.hpp>

#include <shogle/core/error.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace ntf::shogle::res {

model_data::model_data(std::string _path) :
  path(std::move(_path)) {
  Assimp::Importer import;
  const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    throw ntf::error{"[res::model_loader] ASSIMP error: {}", import.GetErrorString()};
  }

  auto dir = file_dir(path);
  auto _load_material = [dir](mesh_data& mesh, aiMaterial* material, aiTextureType type) {
    material_type mat_type;
    switch (type) {
      case aiTextureType_SPECULAR: {
        mat_type = material_type::specular;
        break;
      }
      default: {
        mat_type = material_type::diffuse;
        break;
      }
    }

    for (size_t i = 0; i < material->GetTextureCount(type); ++i) {
      aiString filename;
      material->GetTexture(type, i, &filename);
      auto tex_path = dir + "/" + std::string{filename.C_Str()};
      bool skip {false};

      for (const auto& mat : mesh.materials) {
        if (std::strcmp(mat.first.path.data(), tex_path.data()) == 0) {
          skip = true;
          break;
        }
      }
      if (!skip) {
        mesh.materials.emplace_back(std::make_pair(texture2d_data{tex_path}, mat_type));
      }
    }
  };

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    mesh_data mesh;
    aiMesh* curr_aimesh = scene->mMeshes[i];

    // Extract vertices
    for (size_t j = 0; j < curr_aimesh->mNumVertices; ++j) { 
      vertex3d vert;
      vert.coord = vec3{
        curr_aimesh->mVertices[j].x,
        curr_aimesh->mVertices[j].y,
        curr_aimesh->mVertices[j].z
      };
      vert.normal = vec3{
        curr_aimesh->mNormals[j].x,
        curr_aimesh->mNormals[j].y,
        curr_aimesh->mNormals[j].z
      };

      if (curr_aimesh->mTextureCoords[0]) {
        vert.tex_coord = vec2{
          curr_aimesh->mTextureCoords[0][j].x,
          curr_aimesh->mTextureCoords[0][j].y
        };
      }

      mesh.vertices.emplace_back(std::move(vert));
    }

    // Extract indices
    for (size_t j = 0; j < curr_aimesh->mNumFaces; ++j) {
      aiFace face = curr_aimesh->mFaces[j];
      for (size_t k = 0; k < face.mNumIndices; ++k) {
        mesh.indices.emplace_back(face.mIndices[k]);
      }
    }

    // Extract materials
    if (curr_aimesh->mMaterialIndex > 0) {
      aiMaterial* mat = scene->mMaterials[curr_aimesh->mMaterialIndex];
      _load_material(mesh, mat, aiTextureType_DIFFUSE);
      _load_material(mesh, mat, aiTextureType_SPECULAR);
    }

    // Extract name
    mesh.name = std::string{curr_aimesh->mName.C_Str()};

    meshes.emplace_back(std::move(mesh));
  }
}

model::model(std::string path) :
  model(data_t{std::move(path)}) {}

model::model(data_t data) :
  _path(std::move(data.path)),
  _meshes(data.meshes.size()) {
  using att_coords    = gl::shader_attribute<0, vec3>;
  using att_normals   = gl::shader_attribute<1, vec3>;
  using att_texcoords = gl::shader_attribute<2, vec2>;

  for (size_t i = 0; i < data.meshes.size(); ++i) {
    auto& mesh_data = data.meshes[i];
    auto& _mesh = _meshes[i];

    _mesh.name = std::move(mesh_data.name);
    for (auto& material : mesh_data.materials) {
      _mesh.materials.emplace_back(std::make_pair(
        texture2d{std::move(material.first)},
        std::move(material.second)
      ));
    }
    _mesh.mesh
      .add_vertex_buffer(&mesh_data.vertices[0], mesh_data.vertices.size()*sizeof(vertex3d),
        att_coords{}, att_normals{}, att_texcoords{})
      .add_index_buffer(&mesh_data.indices[0], mesh_data.indices.size()*sizeof(uint));
  }
}

model::mesh& model::find_mesh(std::string name) {
  auto pred = [name](const auto& mesh) { return mesh.name == name; };
  auto it = std::find_if(_meshes.begin(), _meshes.end(), pred);
  if (it != _meshes.end()) {
    return *it;
  }
  throw ntf::error{"[res::model] Mesh not found: {}", name};
}

texture2d& model::mesh::find_material(material_type type) {
  auto pred = [type](const auto& tex_pair) { return tex_pair.second == type; };
  auto it = std::find_if(materials.begin(), materials.end(), pred);
  if (it != materials.end()) {
    return it->first;
  }
  throw ntf::error{"[resoures::model::mesh] Mesh doesn't have provided material"};
}

} // namespace ntf::shogle::res
