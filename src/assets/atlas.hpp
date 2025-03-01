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
  using array_type = unique_array<T, std::default_delete<T>>;

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

class grid_atlas_loader {
public:
  template<typename T>
  using deleter = std::default_delete<T>;

  template<typename T>
  using array_type = unique_array<T, deleter<T>>;

public:
  grid_atlas_loader() noexcept :
    _img_path{}, _sprites{}, _groups{}, _sequences{} {}

public:
  asset_expected<void> parse(const std::string& path);

public:
  std::string image_path();
  array_type<sprite_data> sprites();
  array_type<atlas_index> indices();
  array_type<sprite_data::group> groups();
  optional<array_type<sprite_data::sequence>> sequences();

private:
  std::string _img_path;
  std::vector<sprite_data> _sprites;
  std::vector<sprite_data::group> _groups;
  std::vector<sprite_data::sequence> _sequences;
  std::vector<atlas_index> _indices;
};

} // namespace ntf
