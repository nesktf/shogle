#include <shogle/fs/resources.hpp>
#include <shogle/fs/common.hpp>

#include <shogle/core/error.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <fmt/format.h>

#include <nlohmann/json.hpp>

#include <stb/stb_image.h>

#include <fstream>

namespace ntf::fs {
// texture
texture_loader::texture_loader(std::string _path) :
  path(_path) {
  pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);

  if (!pixels) {
    throw ntf::error(fmt::format("Error loading texture: {}", path));
  }
}

texture_loader::~texture_loader() {
  if (pixels) {
    stbi_image_free(pixels);
  }
}

texture_loader::texture_loader(texture_loader&& t) noexcept :
  path(std::move(t.path)),
  width(std::move(t.width)),
  height(std::move(t.height)),
  channels(std::move(t.channels)),
  t_dim(std::move(t.t_dim)),
  pixels(std::move(t.pixels)) { t.pixels = nullptr; }

texture_loader& texture_loader::operator=(texture_loader&& t) noexcept {
  path = std::move(t.path);
  width = std::move(t.width);
  height = std::move(t.height);
  channels = std::move(t.channels);
  t_dim = std::move(t.t_dim);
  pixels = std::move(t.pixels);

  t.pixels = nullptr;

  return *this;
}

// shader
shader_loader::shader_loader(std::string _path) :
  path(_path),
  vert_src(file_contents(_path+".vs.glsl")),
  frag_src(file_contents(_path+".fs.glsl")) {}

// script
script_loader::script_loader(std::string _path) :
  path(_path),
  content(file_contents(_path)) {}

// material
material_loader::material_loader(std::string path) :
  tex(path) {}

static void _load_materials(std::vector<material_loader>& material_datas, aiMaterial* mat, aiTextureType type, std::string dir) {
  for (size_t i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString filename;
    mat->GetTexture(type, i, &filename);
    std::string tex_path = dir+"/"+std::string{filename.C_Str()};

    bool skip = false;
    for (const auto& material : material_datas) { 
      if (std::strcmp(material.tex.path.data(), tex_path.data()) == 0) {
        skip = true;
        break;
      }
    }
    if (!skip) {
      material_loader mat {tex_path};
      switch (type) {
        case aiTextureType_DIFFUSE: {
          mat.type = material_type::diffuse;
          break;
        }
        case aiTextureType_SPECULAR: {
          mat.type = material_type::specular;
          break;
        }
        default: {
          mat.type = material_type::diffuse;
          break;
        }
      }
      material_datas.emplace_back(std::move(mat));
    }
  }
}

// model
model_loader::model_loader(std::string _path) :
  path(_path) {
  Assimp::Importer import;
  const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    throw ntf::error(fmt::format("ASSIMP: {}", import.GetErrorString()));
  }

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    mesh_data mesh{};
    aiMesh* curr_aimesh = scene->mMeshes[i];

    // Extract vertices
    for (size_t j = 0; j < curr_aimesh->mNumVertices; ++j) { 
      mesh_data::vertex vert{};
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
      std::string dir = file_dir(path);
      _load_materials(mesh.materials, mat, aiTextureType_DIFFUSE, dir);
      _load_materials(mesh.materials, mat, aiTextureType_SPECULAR, dir);
    }

    meshes.push_back(std::move(mesh));
  }
}

// spritesheet
spritesheet_loader::spritesheet_loader(std::string _path) :
  path(_path) {
  using json = nlohmann::json;

  std::ifstream f{path};
  json data = json::parse(f);

  tex = texture_loader {file_dir(path)+"/"+data["file"].template get<std::string>()};

  auto content = data["content"];
  for (auto& sprite : content) {
    sprite_data sp_data {};

    std::string name = sprite["name"].template get<std::string>();

    sp_data.count = sprite["count"].template get<size_t>();
    sp_data.x0 = sprite["x0"].template get<size_t>();
    sp_data.y0 = sprite["y0"].template get<size_t>();
    sp_data.dx = sprite["dx"].template get<size_t>();
    sp_data.dy = sprite["dy"].template get<size_t>();
    sp_data.x = static_cast<size_t>(tex.width);
    sp_data.y = static_cast<size_t>(tex.height);
    sp_data.cols = sprite["cols"].template get<size_t>();
    sp_data.rows = std::ceil((float)sp_data.count/(float)sp_data.cols);

    sprites.emplace(std::make_pair(name, std::move(sp_data)));
  }
}

} // namespace ntf::fs
