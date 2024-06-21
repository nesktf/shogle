#include <shogle/res/loader_data.hpp>
#include <shogle/res/util.hpp>

#include <shogle/core/error.hpp>

#include <stb/stb_image.h>

#include <nlohmann/json.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <fstream>

namespace ntf::shogle {

static tex_format __toenum(int channels) {
  switch (channels) {
    case 1:
      return tex_format::mono;
    case 4:
      return tex_format::rgba;
    default:
      return tex_format::rgb;
  }
}

texture2d_loader::texture2d_loader(std::string_view _path) :
  path(_path) {
  pixels = stbi_load(_path.data(), &width, &height, &channels, 0);
  if (!pixels) {
    throw ntf::error{"[shogle::texture2d_loader] Error loadng texture: {}", _path.data()};
  }
  format = __toenum(channels);
}

texture2d_loader::~texture2d_loader() {
  if (pixels) {
    stbi_image_free(pixels);
  }
}

texture2d_loader::texture2d_loader(texture2d_loader&& t) noexcept :
  path(std::move(t.path)),
  width(std::move(t.width)),
  height(std::move(t.height)),
  channels(std::move(t.channels)),
  format(std::move(t.format)),
  pixels(std::move(t.pixels)) { 
  t.pixels = nullptr; 
}

texture2d_loader& texture2d_loader::operator=(texture2d_loader&& t) noexcept {
  if (pixels) {
    stbi_image_free(pixels);
  }

  path = std::move(t.path);
  width = std::move(t.width);
  height = std::move(t.height);
  channels = std::move(t.channels);
  format = std::move(t.format);
  pixels = std::move(t.pixels);

  t.pixels = nullptr;

  return *this;
}

cubemap_loader::cubemap_loader(std::string_view path) {
  using json = nlohmann::json;
  std::ifstream f{path.data()};

  json data = json::parse(f);
  auto content = data["content"];
  assert(content.size() == CUBEMAP_FACES);

  size_t i = 0;
  for (auto& curr : content) {
    std::string curr_path = file_dir(path.data())+"/"+curr["path"].template get<std::string>();

    // assumes all 6 faces have the same size
    pixels[i] = stbi_load(curr_path.c_str(), &width, &height, &channels, 0);
    if (!pixels[i]) {
      throw ntf::error{"[shogle::cubemap_data] Error loading cubemap texture: {}, n° {}", path, i};
    }

    ++i;
  }

  format = __toenum(channels);
}

cubemap_loader::~cubemap_loader() {
  for (size_t i = 0; i < CUBEMAP_FACES; ++i) {
    if (pixels[i]) {
      stbi_image_free(pixels[i]);
    }
  }
}

cubemap_loader::cubemap_loader(cubemap_loader&& c) noexcept :
  width(std::move(c.width)), 
  height(std::move(c.height)),
  channels(std::move(c.channels)),
  format(std::move(c.format)) {
  for (size_t i = 0; i < CUBEMAP_FACES; ++i) {
    pixels[i] = c.pixels[i];
    c.pixels[i] = nullptr;
  }
}

cubemap_loader& cubemap_loader::operator=(cubemap_loader&& c) noexcept {
  width = std::move(c.width);
  height = std::move(c.height);
  channels = std::move(c.channels);
  format = std::move(c.format);

  for (size_t i = 0; i < CUBEMAP_FACES; ++i) {
    stbi_image_free(pixels[i]);
    pixels[i] = c.pixels[i];
    c.pixels[i] = nullptr;
  }

  return *this;
}


model_loader::model_loader(std::string_view path) {
  Assimp::Importer import;
  const aiScene* scene = import.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    throw ntf::error{"[shogle::model_loader] ASSIMP error: {}", import.GetErrorString()};
  }

  auto dir = file_dir(path.data());
  auto _load_material = [dir](mesh_data& mesh, aiMaterial* aimat, aiTextureType aitype) {
    material_type mat_type;
    switch (aitype) {
      case aiTextureType_SPECULAR: {
        mat_type = material_type::specular;
        break;
      }
      default: {
        mat_type = material_type::diffuse;
        break;
      }
    }

    for (size_t i = 0; i < aimat->GetTextureCount(aitype); ++i) {
      aiString filename;
      aimat->GetTexture(aitype, i, &filename);
      auto tex_path = dir + "/" + std::string{filename.C_Str()};
      bool skip {false};

      for (const auto& mat : mesh.materials) {
        if (std::strcmp(mat.first.path.data(), tex_path.data()) == 0) {
          skip = true;
          break;
        }
      }
      if (!skip) {
        mesh.materials.emplace_back(std::make_pair(texture2d_loader{tex_path}, mat_type));
      }
    }
  };

  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    mesh_data mesh;
    aiMesh* curr_aimesh = scene->mMeshes[i];

    // Extract vertices
    for (size_t j = 0; j < curr_aimesh->mNumVertices; ++j) { 
      vertex3dnt vert;
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

spritesheet_loader::spritesheet_loader(std::string_view path) {
  using json = nlohmann::json;

  std::ifstream f{path.data()};
  json data = json::parse(f);

  auto tex_path = file_dir(path.data())+"/"+data["file"].template get<std::string>();
  texture = texture2d_loader{tex_path};

  auto content = data["content"];
  for (auto& curr_sprite : content) {
    sprite_data sp_data{};

    sp_data.name = curr_sprite["name"].template get<std::string>();
    sp_data.count = curr_sprite["count"].template get<size_t>();
    sp_data.x0 = curr_sprite["x0"].template get<size_t>();
    sp_data.y0 = curr_sprite["y0"].template get<size_t>();
    sp_data.dx = curr_sprite["dx"].template get<size_t>();
    sp_data.dy = curr_sprite["dy"].template get<size_t>();
    sp_data.x = static_cast<size_t>(texture.width);
    sp_data.y = static_cast<size_t>(texture.height);
    sp_data.cols = curr_sprite["cols"].template get<size_t>();
    sp_data.rows = std::ceil((float)sp_data.count/(float)sp_data.cols);

    sprites.emplace_back(std::move(sp_data));
  }
}

} // namespace ntf::shogle
