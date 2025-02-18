#include "./model.hpp"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <unordered_set>

// #define LOG_BONE_THINGIES

namespace ntf {

namespace {

bool is_identity(const mat4& mat) {
  constexpr mat4 id{1.f};
  for (uint32 i = 0; i < 4; ++i) {
    for (uint32 j = 0; j < 4; ++j) {
      if (std::abs(mat[i][j] - id[i][j]) > glm::epsilon<float32>()) {
        return false;
      }
    }
  }
  return true;
}

mat4 asscast(const aiMatrix4x4& mat) {
  mat4 out;
  //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
  out[0][0] = mat.a1; out[1][0] = mat.a2; out[2][0] = mat.a3; out[3][0] = mat.a4;
  out[0][1] = mat.b1; out[1][1] = mat.b2; out[2][1] = mat.b3; out[3][1] = mat.b4;
  out[0][2] = mat.c1; out[1][2] = mat.c2; out[2][2] = mat.c3; out[3][2] = mat.c4;
  out[0][3] = mat.d1; out[1][3] = mat.d2; out[2][3] = mat.d3; out[3][3] = mat.d4;
  return out;
}

vec3 asscast(const aiVector3D& vec) {
  return {vec.x, vec.y, vec.z};
}

color4 asscast(const aiColor4D& col) {
  return {col.r, col.g, col.b, col.a};
}

quat asscast(const aiQuaternion& q) {
  return {q.w, q.x, q.y, q.z};
}

mat4 node_model(const aiNode* node) {
  NTF_ASSERT(node);
  if (node->mParent == nullptr) {
    return asscast(node->mTransformation);
  }
  return node_model(node->mParent)*asscast(node->mTransformation);
}

void parse_bone_nodes(uint32 parent, uint32& bone_counter, const aiNode* node,
                      std::unordered_map<std::string, std::pair<uint32,mat4>>& bone_map,
                      std::vector<armature_data::bone>& bones) {
  NTF_ASSERT(node);
  uint32 node_idx = bone_counter;

  auto it = bone_map.find(node->mName.C_Str());
  NTF_ASSERT(it != bone_map.end());

  bones.emplace_back(
    node->mName.C_Str(), parent,
    asscast(node->mTransformation), it->second.second // local, inv_model
  );
  // Now we can assign an index to the bone map entry
  it->second.first = bone_counter++;

  for (uint32 i = 0; i < node->mNumChildren; ++i) {
    parse_bone_nodes(node_idx, bone_counter, node->mChildren[i], bone_map, bones);
  }
}

void parse_weights(std::unordered_map<std::string, std::pair<uint32,mat4>>& bone_map,
                   const aiMesh* mesh,
                   std::vector<vertex_weights<SHOGLE_ASSIMP_WEIGHTS>>& weights) {
  NTF_ASSERT(!bone_map.empty());
  const size_t mesh_pos = weights.size();
  weights.resize(weights.size()+mesh->mNumVertices);

  auto try_place_weight = [&](uint32 idx, const aiVertexWeight& weight) {
    const size_t pos = mesh_pos+weight.mVertexId;
    NTF_ASSERT(pos < weights.size());
    for (uint32 i = 0; i < SHOGLE_ASSIMP_WEIGHTS; ++i) {
      if (weight.mWeight == 0.f) {
        return;
      }
      auto& bone = weights[pos];
      if (bone.indices[i] == BONE_TOMBSTONE) {
        bone.indices[i] = idx;
        bone.weights[i] = weight.mWeight;
        return;
      }
    }
    SHOGLE_LOG(warning, "[ntf::assimp_loader] Bone weights out of range in vertex {}", pos);
  };

  for (uint32 i = 0; i < mesh->mNumBones; ++i) {
    const aiBone* bone = mesh->mBones[i];
    NTF_ASSERT(bone_map.find(bone->mName.C_Str()) != bone_map.end());
    const uint32 idx = bone_map.at(bone->mName.C_Str()).first;
    for (uint32 j = 0; j < bone->mNumWeights; ++j) {
      try_place_weight(idx, bone->mWeights[j]);
    }
  }
}

#ifdef LOG_BONE_THINGIES
void log_bones(const std::vector<armature_data::armature>& armatures,
               const std::vector<armature_data::bone>& bones,
               std::unordered_map<std::string, std::pair<uint32,mat4>>& bone_map) {
  uint32 i = 0;
  SHOGLE_LOG(verbose, "[ntf::assimp_loader] Bone vec:");
  for (const auto& bone : bones) {
    const char* parent_str = 
      bone.parent == BONE_TOMBSTONE ? "x" : bones[bone.parent].name.c_str();
    SHOGLE_LOG(verbose, " - [{}] {} -> [{}] {}",
               i++, bone.name, bone.parent, parent_str);
    
    const auto& local = bone.local;
    SHOGLE_LOG(verbose, " {} {} {} {}\n    {} {} {} {}\n    {} {} {} {}\n    {} {} {} {}",
               local[0][0], local[1][0], local[2][0], local[3][0],
               local[0][1], local[1][1], local[2][1], local[3][1],
               local[0][2], local[1][2], local[2][2], local[3][2],
               local[0][3], local[1][3], local[2][3], local[3][3]);
    const auto& inv = bone.inv_model;
    SHOGLE_LOG(verbose, " {} {} {} {}\n    {} {} {} {}\n    {} {} {} {}\n    {} {} {} {}",
               inv[0][0], inv[1][0], inv[2][0], inv[3][0],
               inv[0][1], inv[1][1], inv[2][1], inv[3][1],
               inv[0][2], inv[1][2], inv[2][2], inv[3][2],
               inv[0][3], inv[1][3], inv[2][3], inv[3][3]);
  }

  SHOGLE_LOG(verbose, "[ntf::assimp_loader] Bone map:");
  for (const auto& [name, pair] : bone_map) {
    SHOGLE_LOG(verbose, " - [{}] {}", pair.first, name);
  }

  i = 0;
  SHOGLE_LOG(verbose, "[ntf::assimp_loader] Armatures:");
  for (const auto& [name, span]: armatures) {
    SHOGLE_LOG(verbose, " - [{}] {} -> ({}) {} {}",
               i++, name, bones[span.index].name, span.index, span.count);
  }
}
#endif

uint32 parse_process_flags(model_load_flags flags) {
  uint32 out = 0;
  if (+(flags & model_load_flags::triangulate)) {
    out |= aiProcess_Triangulate;
  }
  if (+(flags & model_load_flags::flip_uvs)) {
    out |= aiProcess_FlipUVs;
  }
  if (+(flags & model_load_flags::calc_tangents)) {
    out |= aiProcess_CalcTangentSpace;
  }
  return out;
}

[[nodiscard]] r_material_type asscast(aiTextureType type) {
  switch (type) {
    case aiTextureType_SPECULAR: return r_material_type::specular;
    case aiTextureType_DIFFUSE:  return r_material_type::diffuse;

    default: break;
  }

  // TODO: Handle more material types :p
  NTF_UNREACHABLE();
}

} // namespace

assimp_loader::assimp_loader() {
  _importer = new Assimp::Importer();
}

assimp_loader::~assimp_loader() noexcept {
  Assimp::Importer* importer = static_cast<Assimp::Importer*>(_importer);
  delete importer;
}

asset_expected<void> assimp_loader::parse(const std::string& path, model_load_flags flags) {
  Assimp::Importer* importer = static_cast<Assimp::Importer*>(_importer);
  importer->SetPropertyBool(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, true);

  auto dir = file_dir(path);
  if (!dir) {
    SHOGLE_LOG(error, "[ntf::assimp_loader] Invalid file path: \"{}\"", path);
    return unexpected{asset_error::format({"[ntf::assimp_loader] Invalid file path: \"{}\""},
                                          path)};
  }
  _dir = std::move(*dir);
  _path = path;

  const aiScene* scene = importer->ReadFile(path.c_str(), parse_process_flags(flags));
  if (!scene) {
    SHOGLE_LOG(error, "[ntf::assimp_loader] ASSIMP ERROR: {}", importer->GetErrorString());
    return unexpected{asset_error::format({"ASSIMP ERROR: {}"}, importer->GetErrorString())};
  }

  // Extract all bone names from the model meshes
  for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
    const auto* mesh = scene->mMeshes[i];
    NTF_ASSERT(mesh);
    for (uint32 j = 0; j < mesh->mNumBones; ++j) {
      const auto* bone = mesh->mBones[j];
      NTF_ASSERT(bone);
      if (_bone_map.find(bone->mName.C_Str()) != _bone_map.end()) {
        continue;
      }
      // Don't assign an index yet
      // Also store the inverse model matrix for later use
      _bone_map.emplace(
        std::make_pair(bone->mName.C_Str(),
                       std::make_pair(BONE_TOMBSTONE, asscast(bone->mOffsetMatrix)))
      );
    }
  }

  return {};
}

void assimp_loader::get_armatures(armature_data& arms) {
  Assimp::Importer* importer = static_cast<Assimp::Importer*>(_importer);
  const aiScene* scene = importer->GetScene();
  const aiNode* scene_root = scene->mRootNode;

  if (_bone_map.empty()) {
    SHOGLE_LOG(debug, "[ntf::assimp_loader] No armatures found in \"{}\"", _path);
    return;
  }

  auto& armatures = arms.data;
  auto& bones = arms.bones;

  std::vector<aiNode const*> possible_roots;
  for (const auto& [name, _] : _bone_map) {
    aiNode const* bone_node = scene_root->FindNode(name.c_str());
    NTF_ASSERT(bone_node);
    if (_bone_map.find(bone_node->mParent->mName.C_Str()) != _bone_map.end()) {
      continue;
    }
    // Assume all bone nodes with a parent not present in the map is a root node
    possible_roots.emplace_back(bone_node);
  }

  SHOGLE_LOG(verbose, "[ntf::assimp_loader] Found {} possible bone root(s)",
             possible_roots.size());

  uint32 bone_counter = 0;
  bones.reserve(_bone_map.size());

  for (const auto* root : possible_roots) {
    NTF_ASSERT(root);
    auto it = _bone_map.find(root->mName.C_Str());
    NTF_ASSERT(it != _bone_map.end());

    const auto bone_index = it->second.first;
    if (bone_index != BONE_TOMBSTONE) {
      SHOGLE_LOG(warning,
                 "[ntf::assimp_loader] Bone root \"{}\" already parsed, possible node duplicate",
                 root->mName.C_Str());
      continue;
    }

    uint32 root_idx = bone_counter;
    // Will set BONE_TOMBSTONE as the parent index for the root bone
    parse_bone_nodes(BONE_TOMBSTONE, bone_counter, root, _bone_map, bones);

    if (!is_identity(bones[root_idx].local*bones[root_idx].inv_model)) {
      SHOGLE_LOG(warning,
                 "[ntf::assimp_loader] Malformed transform in root \"{}\", correction applied",
                 root->mName.C_Str());
      bones[root_idx].local = node_model(root->mParent)*bones[root_idx].local;
    }

    armatures.emplace_back(root->mParent->mName.C_Str(),vec_span{root_idx, bone_counter-root_idx});
  }
#ifdef LOG_BONE_THINGIES
  log_bones(armatures, bones, _bone_map);
#endif
  SHOGLE_LOG(debug, "[ntf::assimp_loader] Parsed {} armatures, {} bones from \"{}\"",
             arms.size(), arms.bone_size(), _path);
}

void assimp_loader::get_animations(animation_data& anims) {
  Assimp::Importer* importer = static_cast<Assimp::Importer*>(_importer);
  const aiScene* scene = importer->GetScene();

  if (!scene->HasAnimations()) {
    SHOGLE_LOG(debug, "[ntf::assimp_loader] No animations found in \"{}\"", _path);
    return;
  }

  auto& animations = anims.data;
  auto& kframes = anims.kframes;
  auto& pkeys = anims.pkeys;
  auto& skeys = anims.skeys;
  auto& rkeys = anims.rkeys;

  animations.reserve(scene->mNumAnimations);
  for (uint32 i = 0; i < scene->mNumAnimations; ++i) {
    const aiAnimation* ai_anim = scene->mAnimations[i];
    SHOGLE_LOG(verbose, "[ntf::assimp_loader] Parsing animation \"{}\"", ai_anim->mName.C_Str());

    animations.emplace_back();
    auto& anim = animations.back();
    anim.name = ai_anim->mName.C_Str();
    anim.tps = ai_anim->mTicksPerSecond;
    anim.duration = ai_anim->mDuration;

    uint32 channels = ai_anim->mNumChannels;
    for (uint32 j = 0; j < ai_anim->mNumChannels; ++j) {
      const aiNodeAnim* node_anim = ai_anim->mChannels[j];

      const char* bone_name = node_anim->mNodeName.C_Str();
      auto it = _bone_map.find(bone_name);
      if (it == _bone_map.end()) {
        SHOGLE_LOG(warning, "[ntf::assimp_loader] Unknown bone node \"{}\" for animation \"{}\"",
                   bone_name, anim.name);
        channels--;
        continue;
      }

      kframes.emplace_back();
      auto& frame = kframes.back();
      frame.bone = it->second.first;
      for (uint32  k = 0; k < node_anim->mNumRotationKeys; ++k) {
        const auto& key = node_anim->mRotationKeys[k];
        rkeys.emplace_back(key.mTime, asscast(key.mValue));
      }
      frame.rkeys.index = rkeys.size()-node_anim->mNumRotationKeys;
      frame.rkeys.count = node_anim->mNumRotationKeys;

      for (uint32 k = 0; k < node_anim->mNumScalingKeys; ++k) {
        const auto& key = node_anim->mScalingKeys[k];
        skeys.emplace_back(key.mTime, asscast(key.mValue));
      }
      frame.skeys.index = skeys.size()-node_anim->mNumScalingKeys;
      frame.skeys.count = node_anim->mNumScalingKeys;

      for (uint32 k = 0; k < node_anim->mNumPositionKeys; ++k) {
        const auto& key = node_anim->mPositionKeys[k];
        pkeys.emplace_back(key.mTime, asscast(key.mValue));
      }
      frame.pkeys.index = pkeys.size()-node_anim->mNumPositionKeys;
      frame.pkeys.count = node_anim->mNumPositionKeys;
    }
    anim.frames.index = kframes.size()-channels;
    anim.frames.count = channels;
  }

  SHOGLE_LOG(debug, "[ntf::assimp_loader] Parsed {} animation(s), {} frames from \"{}\"",
             animations.size(), kframes.size(), _path);
}

void assimp_loader::get_materials(material_data& materials) {
  Assimp::Importer* importer = static_cast<Assimp::Importer*>(_importer);
  const aiScene* scene = importer->GetScene();

  if (!scene->HasMaterials()) {
    SHOGLE_LOG(debug, "[ntf::assimp_loader] No materials found in \"{}\"", _path);
    return;
  }

  std::unordered_map<std::string, uint32> parsed_tex;

  auto load_textures = [&](const aiMaterial* ai_mat, aiTextureType type) {
    for (uint32 i = 0; i < ai_mat->GetTextureCount(type); ++i) {
      aiString filename;
      ai_mat->GetTexture(type, i, &filename);
      auto tex_path = fmt::format("{}/{}", _dir,filename.C_Str());

      if (!std::filesystem::exists(tex_path)) {
        SHOGLE_LOG(warning, "[ntf::assimp_loader] Invalid texture path \"{}\"", tex_path);
        continue;
      }

      auto it = parsed_tex.find(tex_path);
      uint32 idx;
      if (it == parsed_tex.end()) {
        SHOGLE_LOG(verbose, "[ntf::assimp_loader] Found texture \"{}\"", tex_path);
        materials.paths.emplace_back(tex_path);
        idx = materials.paths.size()-1;
        parsed_tex.emplace(std::make_pair(std::move(tex_path), idx));
      } else {
        idx = it->second;
      }

      materials.textures.emplace_back(asscast(type), idx);
    }
  };

  materials.data.reserve(scene->mNumMaterials);
  for (uint32 i = 0; i < scene->mNumMaterials; ++i) {
    const aiMaterial* ai_mat = scene->mMaterials[i];

    materials.data.emplace_back();
    auto& mat = materials.data.back();
    mat.name = ai_mat->GetName().C_Str();
    SHOGLE_LOG(verbose, "[ntf::assimp_loader] Found material \"{}\"", mat.name);

    load_textures(ai_mat, aiTextureType_DIFFUSE);
    load_textures(ai_mat, aiTextureType_SPECULAR);
  }

  SHOGLE_LOG(debug,
             "[ntf::assimp_loader] Parsed {} material(s), {} texture(s), {} image(s) from \"{}\"",
             materials.size(), materials.textures.size(), materials.paths.size(), _path);
}

void assimp_loader::parse_meshes(mesh_data<vert_type>& data) {
  Assimp::Importer* importer = static_cast<Assimp::Importer*>(_importer);
  const aiScene* scene = importer->GetScene();

  uint32 vertex_count = 0, index_count = 0;
  auto& meshes = data.data;
  auto& indices = data.indices;
  auto& vertices = data.vertices;

  {
    uint32 pos_count = 0;
    uint32 norm_count = 0;
    uint32 uv_count = 0;
    uint32 tang_count = 0;
    uint32 col_count = 0;
    uint32 weight_count = 0;

    for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
      const aiMesh* mesh = scene->mMeshes[i];
      const uint32 verts = mesh->mNumVertices;

      if (mesh->HasPositions()) {
        pos_count += verts;
      }

      if (mesh->HasNormals()) {
        norm_count += verts;
      }

      if (mesh->HasTextureCoords(0)) {
        uv_count += verts;
      }

      if (mesh->HasTangentsAndBitangents()) {
        tang_count += verts;
      }

      if (mesh->HasVertexColors(0)) {
        col_count += verts;
      }

      if (mesh->HasBones()) {
        weight_count += verts;
      }

      for (uint32 j = 0; j < mesh->mNumFaces; ++j) {
        index_count += mesh->mFaces[j].mNumIndices;
      }
      vertex_count += verts;
    }

    meshes.reserve(scene->mNumMeshes);
    indices.reserve(index_count);

    vertices.positions.reserve(pos_count);
    vertices.normals.reserve(norm_count);
    vertices.uvs.reserve(uv_count);
    vertices.tangents.reserve(tang_count);
    vertices.bitangents.reserve(tang_count);
    vertices.colors.reserve(col_count);
    vertices.weights.reserve(weight_count);
  }

  for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
    meshes.emplace_back();
    auto& mesh = meshes.back();

    const aiMesh* ai_mesh = scene->mMeshes[i];
    mesh.name = ai_mesh->mName.C_Str();

    const uint32 nverts = ai_mesh->mNumVertices;
    if (!ai_mesh->HasPositions()) {
      SHOGLE_LOG(warning, "[ntf::assimp_loader] Mesh without vertices: \"{}\"", mesh.name);
      continue;
    } else {
      SHOGLE_LOG(verbose, "[ntf::assimp_loader] Found {} vertices in mesh \"{}\"", 
                 nverts, mesh.name);
    }

    mesh.material = ai_mesh->mMaterialIndex;
    // mesh.material = ai_mesh->mMaterialIndex > 0 ? ai_mesh->mMaterialIndex : MATERIAL_TOMBSTONE;

    if (ai_mesh->HasFaces()) {
      uint32 mesh_indices = 0;
      for (uint32 j = 0; j < ai_mesh->mNumFaces; ++j) {
        const auto& face = ai_mesh->mFaces[j];
        for (uint32 k = 0; k < face.mNumIndices; ++k) {
          indices.emplace_back(face.mIndices[k]);
        }
        mesh_indices += face.mNumIndices;
      }

      mesh.indices.index = indices.size()-mesh_indices;
      mesh.indices.count = mesh_indices;
      mesh.faces = ai_mesh->mNumFaces;
      SHOGLE_LOG(verbose, "[ntf::assimp_loader] Found {} faces ({} indices) in mesh \"{}\"",
                 ai_mesh->mNumFaces, mesh.indices.count, mesh.name);
    } else {
      mesh.indices.index = VSPAN_TOMBSTONE;
    }

    if (!_bone_map.empty() && ai_mesh->HasBones()) {
      parse_weights(_bone_map, ai_mesh, vertices.weights);
      mesh.weights.index = vertices.weights.size()-nverts;
      mesh.weights.count = nverts;
    } else {
      mesh.weights.index = VSPAN_TOMBSTONE;
    }

    if (ai_mesh->HasNormals()) {
      for (uint32 j = 0; j < nverts; ++j) {
        vertices.normals.emplace_back(asscast(ai_mesh->mNormals[j]));
      }
      mesh.normals.index = vertices.normals.size()-nverts;
      mesh.normals.count = nverts;
    } else {
      mesh.normals.index = VSPAN_TOMBSTONE;
    }

    if (ai_mesh->HasTextureCoords(0)) {
      for (uint32 j = 0; j < nverts; ++j) {
        const auto& uv = ai_mesh->mTextureCoords[0][j];
        vertices.uvs.emplace_back(uv.x, uv.y);
      }
      mesh.uvs.index = vertices.uvs.size()-nverts;
      mesh.uvs.count = nverts;
    } else {
      mesh.uvs.index = VSPAN_TOMBSTONE;
    }

    if (ai_mesh->HasTangentsAndBitangents()) {
      for (uint32 j = 0; j < nverts; ++j) {
        vertices.tangents.emplace_back(asscast(ai_mesh->mTangents[j]));
        vertices.bitangents.emplace_back(asscast(ai_mesh->mBitangents[j]));
      }
      mesh.tangents.index = vertices.tangents.size()-nverts;
      mesh.tangents.count = nverts;
    } else {
      mesh.tangents.index = VSPAN_TOMBSTONE;
    }

    if (ai_mesh->HasVertexColors(0)) {
      for (uint32 j = 0; j < nverts; ++j) {
        vertices.colors.emplace_back(asscast(ai_mesh->mColors[0][j]));
      }
      mesh.colors.index = vertices.colors.size()-nverts;
      mesh.colors.count = nverts;
    } else {
      mesh.colors.index = VSPAN_TOMBSTONE;
    }

    for (uint32 j = 0; j < nverts; ++j) {
      vertices.positions.emplace_back(asscast(ai_mesh->mVertices[j]));
    }
    mesh.positions.index = vertices.positions.size()-nverts;
    mesh.positions.count = nverts;
  }
  SHOGLE_LOG(debug, "[ntf::assimp_loader] Parsed {} vertices, {} indices, {} mesh(es) from \"{}\"",
             vertex_count, index_count, scene->mNumMeshes, _path);
}

} // namespace ntf
