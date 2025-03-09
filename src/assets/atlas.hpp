#pragma once

#include "./texture.hpp"

namespace ntf {

using atlas_index = uint16;

struct sprite_data {
public:
  struct group {
    std::string name;
    vec_span entries;
  };

  struct sequence {
    std::string name;
    vec_span entries;
  };

public:
  vec4 offset;
  uvec2 dim;

public:
  float32 aspect() const { return static_cast<float32>(dim.x)/static_cast<float32>(dim.y); }
  vec2 coord(vec2 uv) const { return vec2{uv.x*offset.x+offset.z, uv.y*offset.y+offset.w}; }
};

struct sprite_atlas_data {
public:
  template<typename T>
  using array_type = unique_array<T>;

  using value_type = sprite_data;
  using iterator = array_type<value_type>::iterator;
  using const_iterator = array_type<value_type>::const_iterator;

public:
  sprite_atlas_data(std::string image_path_,
                    array_type<sprite_data> sprites_) noexcept :
    image_path{std::move(image_path_)},
    sprites{std::move(sprites_)},
    indices{},
    sequences{},
    groups{} {}

  sprite_atlas_data(std::string image_path_,
                    array_type<sprite_data> sprites_,
                    array_type<atlas_index> indices_,
                    array_type<sprite_data::group> groups_) noexcept :
    image_path{std::move(image_path_)},
    sprites{std::move(sprites_)},
    indices{std::move(indices_)},
    sequences{},
    groups{std::move(groups_)} {}

  sprite_atlas_data(std::string image_path_,
                    array_type<sprite_data> sprites_,
                    array_type<atlas_index> indices_,
                    array_type<sprite_data::sequence> sequences_) noexcept :
    image_path{std::move(image_path_)},
    sprites{std::move(sprites_)},
    indices{std::move(indices_)},
    sequences{std::move(sequences_)},
    groups{} {}

  sprite_atlas_data(std::string image_path_,
                    array_type<sprite_data> sprites_,
                    array_type<atlas_index> indices_,
                    array_type<sprite_data::group> groups_,
                    array_type<sprite_data::sequence> sequences_) noexcept :
    image_path{std::move(image_path_)},
    sprites{std::move(sprites_)},
    indices{std::move(indices_)},
    sequences{std::move(sequences_)},
    groups{std::move(groups_)} {}

public:
  value_type& operator[](atlas_index handle) {
    return sprites[static_cast<size_t>(handle)];
  }
  value_type* at(atlas_index handle) {
    return sprites.at(static_cast<size_t>(handle));
  }

  const value_type& operator[](atlas_index handle) const {
    return sprites[static_cast<size_t>(handle)];
  }
  const value_type* at(atlas_index handle) const {
    return sprites.at(static_cast<size_t>(handle));
  }

  iterator begin() { return sprites.begin(); }
  const_iterator begin() const { return sprites.begin(); }
  const_iterator cbegin() const { return sprites.begin(); }

  iterator end() { return sprites.end(); }
  const_iterator end() const { return sprites.end(); }
  const_iterator cend() const { return sprites.end(); }

  sprite_data::sequence& sequence_at(atlas_index handle) {
    return sequences[static_cast<size_t>(handle)];
  }

  const sprite_data::sequence& sequence_at(atlas_index handle) const {
    return sequences[static_cast<size_t>(handle)];
  }

  optional<atlas_index> find_sequence(const std::string& name) const {
    if (sequences.size() == 0) {
      return nullopt;
    }

    const auto it = std::find_if(sequences.begin(), sequences.end(), [&](const auto& seq) {
      return seq.name == name;
    });
    if (it == sequences.end()) {
      return nullopt;
    }

    const auto diff = static_cast<ptrdiff_t>(sequences.end()-it);
    return static_cast<atlas_index>(diff);
  }

  sprite_data::group& group_at(atlas_index handle) {
    return groups[static_cast<size_t>(handle)];
  }

  const sprite_data::group& group_at(atlas_index handle) const {
    return groups[static_cast<size_t>(handle)];
  }

  optional<atlas_index> find_group(const std::string& name) const {
    if (groups.size() == 0) {
      return nullopt;
    }

    const auto it = std::find_if(groups.begin(), groups.end(), [&](const auto& group) {
      return group.name == name;
    });
    if (it == groups.end()) {
      return nullopt;
    }

    const auto diff = static_cast<ptrdiff_t>(groups.end()-it);
    return static_cast<atlas_index>(diff);
  }
  
public:
  std::string image_path;
  array_type<sprite_data> sprites;
  array_type<atlas_index> indices;
  array_type<sprite_data::sequence> sequences;
  array_type<sprite_data::group> groups;
};

enum class atlas_load_flags {
  none = 0,
  flip_y = 1 << 0,
  flip_x = 1 << 1,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(atlas_load_flags);

template<typename Loader>
concept checked_atlas_loader_type = requires(Loader loader,
                                             const std::string& path,
                                             atlas_load_flags flags,
                                             typename Loader::data_t& data) {
  { loader.parse(path, flags) } -> std::same_as<asset_expected<typename Loader::data_t>>;
  { loader.image_path(data) } -> std::same_as<std::string>;
  { loader.sprites(data) } -> std::same_as<sprite_atlas_data::template array_type<sprite_data>>;
};

template<typename Loader>
concept unchecked_atlas_loader_type = requires(Loader loader,
                                               const std::string& path,
                                               atlas_load_flags flags,
                                               typename Loader::data_t& data) {
  { loader.parse(unchecked_t{}, path, flags) } -> std::same_as<typename Loader::data_t>;
  { loader.image_path(data) } -> std::same_as<std::string>;
  { loader.sprites(data) } -> std::same_as<sprite_atlas_data::template array_type<sprite_data>>;
};

template<typename T>
concept atlas_loader_type = checked_atlas_loader_type<T> || unchecked_atlas_loader_type<T>;

template<typename Loader>
concept atlas_loader_with_groups = atlas_loader_type<Loader> &&
                                   requires(Loader loader,
                                            typename Loader::data_t& data) {
  { loader.indices(data) } -> std::same_as<sprite_atlas_data::template array_type<atlas_index>>;
  { loader.groups(data) }
    -> same_as_any<
      sprite_atlas_data::template array_type<sprite_data::group>,
      optional<sprite_atlas_data::template array_type<sprite_data::group>>
    >;
};

template<typename Loader>
concept atlas_loader_with_sequences = atlas_loader_type<Loader> &&
                                      requires(Loader loader,
                                               typename Loader::data_t data) {
  { loader.indices(data) } -> std::same_as<sprite_atlas_data::template array_type<atlas_index>>;
  { loader.sequences(data) }
    -> same_as_any<
      sprite_atlas_data::template array_type<sprite_data::sequence>,
      optional<sprite_atlas_data::template array_type<sprite_data::sequence>>
    >;
};

namespace impl {

template<atlas_loader_type Loader, bool checked>
auto load_atlas(
  const std::string& path,
  atlas_load_flags flags,
  Loader&& loader
) -> std::conditional_t<checked, asset_expected<sprite_atlas_data>, sprite_atlas_data> {
  using atlas_data_t = sprite_atlas_data;
  using ret_t = std::conditional_t<checked, asset_expected<atlas_data_t>, atlas_data_t>;

  // TODO: Maybe rewrite this unholy mess, somehow
  auto parse_ret_data = [&](auto&& data) -> ret_t {
    if constexpr (atlas_loader_with_groups<Loader> && atlas_loader_with_sequences<Loader>) {
      auto groups = loader.groups(data);
      auto seqs = loader.sequences(data);
      using gret_t = decltype(groups);
      using sret_t = decltype(seqs);
      if constexpr (is_optional_type<gret_t> && is_optional_type<sret_t>) {
        if (groups && seqs) {
          return atlas_data_t {
            loader.image_path(data),
            loader.sprites(data),
            loader.indices(data),
            std::move(*groups),
            std::move(*seqs)
          };
        } else if (groups) {
          return atlas_data_t {
            loader.image_path(data),
            loader.sprites(data),
            loader.indices(data),
            std::move(*groups),
          };
        } else if (seqs) {
          return atlas_data_t {
            loader.image_path(data),
            loader.sprites(data),
            loader.indices(data),
            std::move(*seqs),
          };
        } else {
          return atlas_data_t {
            loader.image_path(data),
            loader.sprites(data),
          };
        }
      } else if constexpr (is_optional_type<gret_t>) {
        if (groups) {
          return atlas_data_t {
            loader.image_path(data),
            loader.sprites(data),
            loader.indices(data),
            std::move(*groups),
            std::move(seqs)
          };
        } else {
          return atlas_data_t {
            loader.image_path(data),
            loader.sprites(data),
            loader.indices(data),
            std::move(seqs)
          };
        }
      } else if constexpr (is_optional_type<sret_t>) {
        if (seqs) {
          return atlas_data_t {
            loader.image_path(data),
            loader.sprites(data),
            loader.indices(data),
            std::move(groups),
            std::move(*seqs)
          };
        } else {
          return atlas_data_t {
            loader.image_path(data),
            loader.sprites(data),
            loader.indices(data),
            std::move(groups),
          };
        }
      } else {
        return atlas_data_t {
          loader.image_path(data),
          loader.sprites(data),
          loader.indices(data),
          std::move(groups),
          std::move(seqs)
        };
      }
    }
  };

  if constexpr (checked) {
    return asset_expected<atlas_data_t>::catch_error([&]() -> asset_expected<atlas_data_t> {
      if constexpr (checked_atlas_loader_type<Loader>) {
        return loader.parse(path, flags).and_then(parse_ret_data);
      } else {
        return parse_ret_data(loader.parse(unchecked, path, flags));
      }
    });
  } else if constexpr (unchecked_atlas_loader_type<Loader>) {
    return parse_ret_data(loader.parse(unchecked, path, flags));
  } else {
    auto ret = loader.parse(path, flags);
    NTF_ASSERT(ret);
    return parse_ret_data(std::move(*ret));
  }
}

} // namespace impl

class grid_atlas_loader {
public:
  template<typename T>
  using array_type = unique_array<T>;

  struct data_t {
    std::string img_path;
    array_type<sprite_data> sprites;
    array_type<sprite_data::group> groups;
    array_type<sprite_data::sequence> sequences;
    std::vector<atlas_index> indices;
  };

public:
  grid_atlas_loader() = default;

public:
  asset_expected<data_t> parse(const std::string& path, atlas_load_flags flags);
  data_t parse(unchecked_t, const std::string& path, atlas_load_flags flags);

public:
  std::string image_path(data_t& data);
  array_type<sprite_data> sprites(data_t& data);
  array_type<sprite_data::group> groups(data_t& data);
  optional<array_type<sprite_data::sequence>> sequences(data_t& data);
  array_type<atlas_index> indices(data_t& data);
};
static_assert(checked_atlas_loader_type<grid_atlas_loader>);
static_assert(unchecked_atlas_loader_type<grid_atlas_loader>);
static_assert(atlas_loader_with_sequences<grid_atlas_loader>);
static_assert(atlas_loader_with_groups<grid_atlas_loader>);

template<atlas_loader_type Loader = grid_atlas_loader>
asset_expected<sprite_atlas_data> load_atlas(
  const std::string& path,
  atlas_load_flags flags = atlas_load_flags::none,
  Loader&& loader = {}
) {
  return impl::load_atlas<Loader, true>(path, flags, std::forward<Loader>(loader));
}

template<atlas_loader_type Loader = grid_atlas_loader>
sprite_atlas_data load_atlas(
  unchecked_t,
  const std::string& path,
  atlas_load_flags flags = atlas_load_flags::none,
  Loader&& loader = {}
) {
  return impl::load_atlas<Loader, false>(path, flags, std::forward<Loader>(loader));
}

} // namespace ntf
