#include <shogle/res/assets.hpp>
#include <shogle/res/util.hpp>

#include <shogle/core/error.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <fmt/format.h>

#include <nlohmann/json.hpp>

#include <stb/stb_image.h>

#include <fstream>

namespace ntf::res {

// texture
texture_loader::texture_loader(std::string _path) :
  path(_path) {
  pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);

  if (!pixels) {
    throw ntf::error{"Error loading texture: {}", path};
  }

  switch (channels) {
    case 1: {
      format = texture_format::mono;
      break;
    }
    case 4: {
      format = texture_format::rgba;
      break;
    }
    default: {
      format = texture_format::rgb;
      break;
    }
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
  type(std::move(t.type)),
  format(std::move(t.format)),
  filter(std::move(t.filter)),
  pixels(std::move(t.pixels)) { t.pixels = nullptr; }

texture_loader& texture_loader::operator=(texture_loader&& t) noexcept {
  path = std::move(t.path);
  width = std::move(t.width);
  height = std::move(t.height);
  channels = std::move(t.channels);
  type = std::move(t.type);
  format = std::move(t.format);
  filter = std::move(t.filter);
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

// model
model_loader::model_loader(std::string _path) :
  path(_path) {
  Assimp::Importer import;
  const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    throw ntf::error{"[model_loader] ASSIMP error: {}", import.GetErrorString()};
  }

  auto _load_material_type = [this](mesh& mesh, aiMaterial* material, aiTextureType type) {
    auto dir = file_dir(path);
    size_t material_count = 0;
    for (size_t i = 0; i < material->GetTextureCount(type); ++i) {
      aiString filename;
      material->GetTexture(type, i, &filename);

      auto path = dir + "/" + std::string{filename.C_Str()};
      bool skip = false;

      for (const auto& mat : mesh.materials) {
        if (std::strcmp(mat.texture.path.data(), path.data()) == 0) {
          skip = true;
          break;
        }
      }
      if (!skip) {
        std::string uniform_name;
        switch (type) {
          case aiTextureType_SPECULAR: {
            uniform_name = "material.specular";
            break;
          }
          default: {
            uniform_name = "material.diffuse";
            break;
          }
        }
        uniform_name += std::to_string(material_count++);
        mesh.materials.emplace_back(texture_loader{path}, uniform_name);
      }

    }

  };

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    mesh mesh;
    aiMesh* curr_aimesh = scene->mMeshes[i];

    // Extract vertices
    for (size_t j = 0; j < curr_aimesh->mNumVertices; ++j) { 
      mesh::vertex vert;
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
      _load_material_type(mesh, mat, aiTextureType_DIFFUSE);
      _load_material_type(mesh, mat, aiTextureType_SPECULAR);
    }

    meshes.emplace_back(std::move(mesh));
  }
}

// spritesheet
spritesheet_loader::spritesheet_loader(std::string _path) :
  path(_path) {
  using json = nlohmann::json;

  std::ifstream f{path};
  json data = json::parse(f);

  tex = texture_loader{file_dir(path)+"/"+data["file"].template get<std::string>()};

  auto content = data["content"];
  for (auto& curr_sprite : content) {
    sprite sp_data {};

    std::string name = curr_sprite["name"].template get<std::string>();

    sp_data.count = curr_sprite["count"].template get<size_t>();
    sp_data.x0 = curr_sprite["x0"].template get<size_t>();
    sp_data.y0 = curr_sprite["y0"].template get<size_t>();
    sp_data.dx = curr_sprite["dx"].template get<size_t>();
    sp_data.dy = curr_sprite["dy"].template get<size_t>();
    sp_data.x = static_cast<size_t>(tex.width);
    sp_data.y = static_cast<size_t>(tex.height);
    sp_data.cols = curr_sprite["cols"].template get<size_t>();
    sp_data.rows = std::ceil((float)sp_data.count/(float)sp_data.cols);

    sprites.emplace(std::make_pair(name, std::move(sp_data)));
  }

  sprites.emplace(std::make_pair("__sheet", sprite{ 
    .count = 1,
    .x = static_cast<size_t>(tex.width),
    .y = static_cast<size_t>(tex.height),
    .x0 = 0,
    .y0 = 0,
    .dx = static_cast<size_t>(tex.width),
    .dy = static_cast<size_t>(tex.height),
    .cols = 1,
    .rows = 1
  }));
}

} // namespace ntf::res
