#pragma once

#include "../render/vertex.hpp"
#include "./texture.hpp"

#include "../math/quaternion.hpp"

namespace ntf {

template<typename Vertex>
struct mesh_data {
  struct mesh {
    std::string name;
    uint32 material;

    uint32 faces;
    vec_span indices;

    vec_span vertices;

    size_t vertex_count() const { return vertices.count; }
    size_t index_count() const { return indices.count; }

    bool has_indices() const { return indices.index != VSPAN_TOMBSTONE; }
    bool has_vertices() const { return vertices.index != VSPAN_TOMBSTONE; }

    size_t vertices_size() const { return vertex_count()*sizeof(Vertex); }
    size_t indices_size() const { return index_count()*sizeof(uint32); }

    Vertex* vertex_data(std::vector<Vertex>& verts) const {
      NTF_ASSERT(vertices.index < verts.size());
      return &verts[vertices.index];
    }

    const Vertex* vertex_data(const std::vector<Vertex>& verts) const {
      NTF_ASSERT(vertices.index < verts.size());
      return &verts[vertices.index];
    }

    uint32* index_data(std::vector<uint32>& inds) const {
      NTF_ASSERT(indices.index < inds.size());
      return &inds[indices.index];
    }

    const uint32* index_data(const std::vector<uint32>& inds) const {
      NTF_ASSERT(indices.index < inds.size());
      return &inds[indices.index];
    }
  };

  std::vector<mesh> data;
  std::vector<Vertex> vertices;
  std::vector<uint32> indices;

  size_t size() const { return data.size(); }

  size_t vertex_count() const { return vertices.size(); }

  bool has_vertices() const { return !vertices.empty(); }
  bool has_indices() const { return !indices.empty(); }
};

template<size_t num_weights>
struct mesh_data<soa_vertices<num_weights>> {
  struct mesh {
    std::string name;
    uint32 material;

    uint32 faces;
    vec_span indices;

    vec_span positions;
    vec_span normals;
    vec_span uvs;
    vec_span tangents; // bitangents too
    vec_span weights;
    vec_span colors;

    // Should always have at least one position per vertex
    size_t vertex_count() const { return positions.count; }
    size_t index_count() const { return indices.count; }

    bool has_indices() const { return indices.index != VSPAN_TOMBSTONE; }
    bool has_positions() const { return positions.index != VSPAN_TOMBSTONE; }
    bool has_normals() const { return normals.index != VSPAN_TOMBSTONE; }
    bool has_uvs() const { return uvs.index != VSPAN_TOMBSTONE; }
    bool has_tangents() const { return tangents.index != VSPAN_TOMBSTONE; }
    bool has_weights() const { return weights.index != VSPAN_TOMBSTONE; }
    bool has_colors() const { return colors.index != VSPAN_TOMBSTONE; }

    size_t indices_size() const { return index_count()*sizeof(uint32); }
  };

  std::vector<mesh> data;
  soa_vertices<num_weights> vertices;
  std::vector<uint32> indices;

  size_t size() const { return data.size(); }
  size_t vertex_count() const { return vertices.positions.size(); }

  bool has_positions() const { return !vertices.positions.empty(); }
  bool has_normals() const { return !vertices.normals.empty(); }
  bool has_uvs() const { return !vertices.uvs.empty(); }
  bool has_tangents() const { return !vertices.tangents.empty(); }
  bool has_colors() const { return !vertices.colors.empty(); }
  bool has_weights() const { return !vertices.weights.empty(); }
};

struct armature_data {
  struct armature {
    std::string name;
    vec_span bones;
  };
  struct bone {
    std::string name;
    uint32 parent;
    mat4 local;
    mat4 inv_model;
  };

  std::vector<armature> data;
  std::vector<bone> bones;

  size_t size() const { return data.size(); }
  size_t bone_size() const { return bones.size(); }
};

struct animation_data {
  struct animation {
    std::string name;
    float64 duration;
    float64 tps;
    vec_span frames;
  };

  struct key_frame {
    uint32 bone;
    vec_span pkeys;
    vec_span rkeys;
    vec_span skeys;
  };

  template<typename T>
  using key_t = std::pair<float64, T>;

  std::vector<animation> data;
  std::vector<key_frame> kframes;
  std::vector<key_t<vec3>> pkeys;
  std::vector<key_t<vec3>> skeys;
  std::vector<key_t<quat>> rkeys;

  size_t size() const { return data.size(); }
};

struct material_data {
  struct material {
    std::string name;
    vec_span textures;
  };
  struct texture_entry {
    ntf::r_material_type type;
    uint32 index;
  };

  std::vector<material> data;
  std::vector<texture_entry> textures;
  std::vector<std::string> paths;

  size_t size() const { return data.size(); }
};

template<typename Vertex>
struct model_data {
  armature_data armatures;
  animation_data animations;
  material_data materials;
  mesh_data<Vertex> meshes;

  bool has_armatures() const { return armatures.size() != 0; }
  bool has_animations() const { return animations.size() != 0; }
  bool has_materials() const { return materials.size() != 0; }
  bool has_meshes() const { return meshes.size() != 0; }
};

enum class model_load_flags {
  none          = 0,
  triangulate   = 1 << 0,
  flip_uvs      = 1 << 1,
  calc_tangents = 1 << 2,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(model_load_flags);

namespace impl {

template<typename T>
concept model_loader_parse = requires(T loader,
                                      const std::string& path,
                                      model_load_flags flags) {
  { loader.parse(path, flags) } -> same_as_any<void, asset_expected<void>>;
};

template<typename T>
concept model_loader_guarded_parse = requires(T loader,
                                              const std::string& path,
                                              model_load_flags flags) {
  { loader.parse(path, flags) } -> std::same_as<asset_expected<void>>;
};

template<typename T, typename Vertex>
concept model_loader_meshes = requires(T loader,
                                       mesh_data<Vertex>& data
) {
  { loader.get_meshes(data) } -> std::same_as<void>;
};

template<typename T>
concept model_loader_armatures = requires(T loader,
                                          armature_data& data
) {
  { loader.get_armatures(data) } -> std::same_as<void>;
};

template<typename T>
concept model_loader_animations = requires(T loader,
                                           animation_data& data
) {
  { loader.get_animations(data) } -> std::same_as<void>;
};

template<typename T>
concept model_loader_materials = requires(T loader,
                                          material_data& data
) {
  { loader.get_materials(data) } -> std::same_as<void>;
};

template<typename T, typename Vertex>
concept model_loader =
  (model_loader_parse<T> || model_loader_guarded_parse<T>) && model_loader_meshes<T, Vertex>;

template<typename Vertex, bool>
struct load_model_ret {
  using type = asset_expected<model_data<Vertex>>;
};

template<typename Vertex>
struct load_model_ret<Vertex, false> {
  using type = model_data<Vertex>;
};

template<typename Vertex, model_loader<Vertex> Loader, bool checked>
load_model_ret<Vertex, checked>::type load_model(const std::string& path,
                                                 model_load_flags flags,
                                                 Loader&& loader) {
  using model_t = model_data<Vertex>;
  auto make_data = [&]() {
    // Assume everything else doesn't throw
    model_data<Vertex> model;

    if constexpr (model_loader_armatures<Loader>) {
      loader.get_armatures(model.armatures);
    }

    if constexpr (model_loader_animations<Loader>) {
      loader.get_animations(model.animations);
    }

    if constexpr (model_loader_materials<Loader>) {
      loader.get_materials(model.materials);
    }

    loader.get_meshes(model.meshes);

    return model;
  };

  if constexpr (checked) {
    return asset_expected<model_t>::catch_error([&]() -> asset_expected<model_t> {
      if constexpr (model_loader_guarded_parse<Loader>) {
        auto ret = loader.parse(path, flags);
        if (!ret) {
          return unexpected{std::move(ret.error())};
        }
      } else {
        loader.parse(path, flags);
      }
      return make_data();
    });
  } else {
    if constexpr (model_loader_guarded_parse<Loader>) {
      auto ret = loader.parse(path, flags);
      NTF_ASSERT(ret);
    } else {
      loader.parse(path, flags);
    }
    return make_data();
  }
}

} // namespace impl

template<is_aos_vertex Vertex, size_t num_weights>
mesh_data<Vertex> soa_to_aos(const mesh_data<soa_vertices<num_weights>>& data) {
  mesh_data<Vertex> out;
  auto& out_verts = out.vertices;
  out.data.reserve(data.size());
  out_verts.resize(data.vertex_count());

  uint32 offset = 0;
  const auto& in_verts = data.vertices;
  for (const auto& mesh : data.data) {
    NTF_ASSERT(mesh.has_positions());
    const size_t mesh_verts = mesh.vertex_count();
    {
      uint32 count = 0;
      mesh.positions.for_each(in_verts.positions, [&](const vec3& pos) {
        out_verts[offset+count].set_position(pos);
        ++count;
      });
    }

    if constexpr (vert_has_normals<Vertex>) {
      if (mesh.has_normals()) {
        uint32 count = 0;
        NTF_ASSERT(mesh.normals.count == mesh_verts);
        mesh.normals.for_each(in_verts.normals, [&](const vec3& norm) {
          out_verts[offset+count].set_normal(norm);
          ++count;
        });
      } else {
        const vec3 def{0.f, 0.f, 0.f};
        for (uint32 count = 0; count < mesh_verts; ++count) { 
          out_verts[offset+count].set_normal(def);
        }
      }
    }

    if constexpr (vert_has_uvs<Vertex>) {
      if (mesh.has_uvs()) {
        uint32 count = 0;
        NTF_ASSERT(mesh.uvs.count == mesh_verts);
        mesh.uvs.for_each(in_verts.uvs, [&](const vec2& uv) {
          out_verts[offset+count].set_uv(uv);
          ++count;
        });
      } else {
        const vec2 def{0.f, 0.f};
        for (uint32 count = 0; count < mesh_verts; ++count) { 
          out_verts[offset+count].set_uv(def);
        }
      }
    }

    if constexpr (vert_has_tangents<Vertex>) {
      if (mesh.has_tangents()) {
        uint32 count = 0;
        NTF_ASSERT(mesh.tangents.count == mesh_verts);
        mesh.tangents.for_each(in_verts.tangents, [&](const vec3& tang) {
          out_verts[offset+count].set_tangent(tang);
          ++count;
        });
        count = 0;
        mesh.bitangents.for_each(in_verts.bitangents, [&](const vec3& bitang) {
          out_verts[offset+count].set_bitangent(bitang);
          ++count;
        });
      } else {
        const vec3 def{0.f, 0.f, 0.f};
        for (uint32 count = 0; count < mesh_verts; ++count) { 
          out_verts[offset+count].set_tangent(def);
          out_verts[offset+count].set_bitangent(def);
        }
      }
    }

    if constexpr (vert_has_colors<Vertex>) {
      if (mesh.has_colors()) {
        uint32 count = 0;
        NTF_ASSERT(mesh.colors.count == mesh_verts);
        mesh.colors.for_each(in_verts.colors, [&](const color4& col) {
          out_verts[offset+count].set_color(col);
          ++count;
        });

      } else {
        const color4 def{0.f, 0.f, 0.f, 0.f};
        for (uint32 count = 0; count < mesh_verts; ++count) { 
          out_verts[offset+count].set_color(def);
        }
      }
    }

    if constexpr (vert_has_weights<Vertex, num_weights>) {
      if (mesh.has_weights()) {
        uint32 count = 0;
        NTF_ASSERT(mesh.weights.count == mesh_verts);
        mesh.weights.for_each(in_verts.weights, [&](const vertex_weights<num_weights>& weights) {
          out_verts[offset+count].set_weights(weights);
          ++count;
        });
      } else {
        const vertex_weights<num_weights> def;
        for (uint32 count = 0; count < mesh_verts; ++count) { 
          out_verts[offset+count].set_weights(def);
        }
      }
    }

    out.data.emplace_back(
      mesh.name, mesh.material, mesh.faces, mesh.indices, mesh.positions
    );
    out.indices = data.indices;

    offset += mesh_verts;
  }

  return out;
}

class assimp_loader {
public:
  using vert_type = soa_vertices<SHOGLE_ASSIMP_WEIGHTS>;

public:
  assimp_loader();
  ~assimp_loader() noexcept;

  asset_expected<void> parse(const std::string& path, model_load_flags flags);

  void get_armatures(armature_data& data);
  void get_animations(animation_data& data);
  void get_materials(material_data& data);

  template<typename Vertex>
  void get_meshes(mesh_data<Vertex>& data) {
    if constexpr (is_soa_vertex<Vertex>) {
      static_assert(std::is_same_v<Vertex, vert_type>, "Invalid vertex type");
      return parse_meshes(data);
    } else {
      mesh_data<vert_type> meshes;
      parse_meshes(meshes);
      data = soa_to_aos<Vertex>(meshes);
    }
  }

private:
  void parse_meshes(mesh_data<vert_type>& data);

private:
  void* _importer;
  std::unordered_map<std::string, std::pair<uint32,mat4>> _bone_map;
  std::string _path, _dir;
};

static_assert(impl::model_loader<assimp_loader, assimp_loader::vert_type>);
static_assert(impl::model_loader_materials<assimp_loader>);
static_assert(impl::model_loader_animations<assimp_loader>);
static_assert(impl::model_loader_armatures<assimp_loader>);

template<
  typename Vertex = soa_vertices<SHOGLE_ASSIMP_WEIGHTS>, 
  impl::model_loader<Vertex> Loader = assimp_loader
>
model_data<Vertex> load_model(
  unchecked_t,
  const std::string& path,
  model_load_flags flags,
  Loader&& loader = {}
) {
  return impl::load_model<Vertex, Loader, false>(path, flags, std::forward<Loader>(loader));
}

template<
  typename Vertex = soa_vertices<SHOGLE_ASSIMP_WEIGHTS>,
  impl::model_loader<Vertex> Loader = assimp_loader
>
asset_expected<model_data<Vertex>> load_model(
  const std::string& path, 
  model_load_flags flags,
  Loader&& loader = {}
) {
  return impl::load_model<Vertex, Loader, true>(path, flags, std::forward<Loader>(loader));
}

} // namespace ntf
