#pragma once

#include <shogle/assets/common.hpp>
#include <shogle/assets/texture.hpp>

namespace ntf {

template<typename Texture>
class texture_atlas {
public:
  using texture_type = Texture;

  using texture_handle = uint16_t;
  using group_handle = uint16_t;
  using sequence_handle = uint16_t;

  using texture_vec = std::vector<texture_handle>;

  struct texture_meta {
    vec4 offset;
    ivec2 dim;
    vec2 aspect() const { return vec2{(float)dim.x/(float)dim.y, 1.f};}
  };

  struct data_type {
  public:
    using texture_type = Texture;
    using texture_data_type = texture_data<texture_type>;

    struct loader {
      texture_atlas operator()(data_type data);
      texture_atlas operator()(std::string path, tex_filter filter, tex_wrap wrap);
    };

  public:
    data_type(std::string_view path, tex_filter filter, tex_wrap wrap);

  public:
    texture_data_type texture;
    std::vector<texture_meta> metas;
    std::vector<std::pair<std::string, texture_vec>> sequences;
    std::vector<std::pair<std::string, texture_vec>> groups;
  };

  // using iterator = std::vector<texture_meta>::iterator;
  using const_iterator = std::vector<texture_meta>::const_iterator;

public:
  texture_atlas() = default;

  texture_atlas(texture_type tex) :
    _texture(std::move(tex)),  _metas{def_meta(_texture.dim())} {} // Just wrap a texture

  texture_atlas(texture_type texture, std::vector<texture_meta> metas,
                std::vector<std::pair<std::string, texture_vec>> groups = {},
                std::vector<std::pair<std::string, texture_vec>> sequences = {});

public:
  const texture_meta& at(texture_handle tex) const;
  const texture_meta& operator[](texture_handle tex) const;

  const texture_vec& sequence_at(sequence_handle seq) const;
  const texture_vec& group_at(group_handle group) const;
  std::optional<sequence_handle> find_sequence(std::string_view name) const;
  std::optional<group_handle> find_group(std::string_view name) const;

  const texture_type& texture() const { return _texture; }
  operator const texture_type&() const { return _texture; }
  texture_type& texture() { return _texture; }
  operator texture_type&() { return _texture; }

  size_t size() const { return _metas.size(); }
  size_t group_count() const { return _groups.size(); }
  size_t sequence_count() const { return _sequences.size(); }

  bool valid() const { return _texture.valid() && _metas.size() > 0; }

  // iterator begin() { return _metas.begin(); }
  const_iterator begin() const { return _metas.begin(); }
  const_iterator cbegin() const { return _metas.cbegin(); }

  // iterator end() { return _metas.end(); }
  const_iterator end() const { return _metas.end(); }
  const_iterator cend() const { return _metas.cend(); }

public:
  static texture_meta def_meta(ivec2 dim) { return texture_meta{.offset=vec4{vec2{1.f},vec2{0.f}},.dim=dim}; }

private:
  texture_type _texture;
  std::vector<texture_meta> _metas;

  std::vector<texture_vec> _groups;
  std::unordered_map<std::string, group_handle> _group_names;

  std::vector<texture_vec> _sequences;
  std::unordered_map<std::string, sequence_handle> _sequence_names;
};


template<typename Texture, typename AtlasPtr = texture_atlas<Texture>*>
class texture_animator {
public:
  using texture_type = Texture;
  using atlas_type = texture_atlas<Texture>;
  using sequence_handle = texture_atlas<Texture>::sequence_handle;
  using texture_handle = texture_atlas<Texture>::texture_handle;

private:
  struct entry {
    sequence_handle sequence;
    uint duration, clock{0};
  };

public:
  texture_animator() = default;

  texture_animator(const AtlasPtr atlas, sequence_handle first_sequence) :
    _atlas(atlas) { enqueue_sequence(first_sequence, 0); }

public:
  void enqueue_sequence(sequence_handle sequence, uint loops);
  void enqueue_sequence_frames(sequence_handle sequence, uint frames);
  void soft_switch(sequence_handle sequence, uint loops);
  void hard_switch(sequence_handle sequence, uint loops);

public:
  void tick();
  texture_handle frame() const;
  AtlasPtr atlas() const { return _atlas; }

  bool valid() const { return _atlas->valid(); }

private:
  void reset_queue(bool hard);
  
private:
  AtlasPtr _atlas;
  std::queue<entry> _sequences;
};

} // namespace ntf

#ifndef SHOGLE_ASSETS_ATLAS_INL
#include <shogle/assets/atlas.inl>
#endif
