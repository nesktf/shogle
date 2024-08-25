#pragma once

#include <shogle/assets/texture.hpp>

namespace ntf {

template<typename Texture>
class texture_atlas {
public:
  using texture_type = Texture;
  using texture = uint32_t;
  struct texture_meta;

  using group = std::vector<texture>;
  using sequence = std::vector<texture>;

  struct data_type;

public:
  texture_atlas() = default;

  texture_atlas(texture_type tex) :
    _texture(std::move(tex)),  _metas{default_meta(_texture.dim())} {} // Just wrap a texture

  texture_atlas(texture_type tex, std::vector<texture_meta> sprites, 
                strmap<group> groups = {}, strmap<sequence> seq = {}) :
    _texture(std::move(tex)), _metas(std::move(sprites)),
    _groups(std::move(groups)), _sequences(std::move(seq)) {}

public:
  const sequence& sequence_at(std::string_view name) const { return _sequences.at(name.data()); }
  const group& group_at(std::string_view name) const { return _groups.at(name.data()); }

  const texture_meta& at(texture id) const { return _metas.at(id); }
  const texture_meta& operator[](texture id) const { return _metas[id]; }

  const texture_type& tex() const { return _texture; }
  texture_type& tex() { return _texture; }

  size_t size() const { return _metas.size(); }
  bool valid() const { return _texture.valid() && _metas.size() > 0; }
  bool has(texture id) const { return _metas.size() > id; }

  operator const texture_type&() const { return _texture; }

public:
  texture_meta default_meta(ivec2 dim) { return texture_meta{.offset=vec4{vec2{1.f},vec2{0.f}},.dim=dim}; }

private:
  texture_type _texture;
  std::vector<texture_meta> _metas;
  strmap<group> _groups;
  strmap<sequence> _sequences;
};


template<typename Texture>
struct texture_atlas<Texture>::texture_meta {
public:
  vec2 aspect() const { return vec2{(float)dim.x/(float)dim.y, 1.f};}

public:
  vec4 offset;
  ivec2 dim;
};


template<typename Texture>
struct texture_atlas<Texture>::data_type {
public:
  using texture_type = Texture;
  using texture_data_type = texture_data<texture_type>;

  struct loader {
    texture_atlas operator()(data_type data) {
      typename texture_type::loader tex_loader;
      auto& tdata = data.texture;
      auto tex = tex_loader(tdata.pixels, tdata.dim, tdata.format, tdata.filter, tdata.wrap);
      return texture_atlas{std::move(tex), std::move(data.sprites), 
        std::move(data.groups), std::move(data.sequences)};
    }
    texture_atlas operator()(std::string_view path, tex_filter filter, tex_wrap wrap) {
      return (*this)(data_type{path, filter, wrap});
    }
  };

public:
  data_type(std::string_view path, tex_filter filter, tex_wrap wrap);

public:
  texture_data_type texture;
  std::vector<texture_atlas::texture_meta> sprites;
  strmap<texture_atlas::sequence> sequences;
  strmap<texture_atlas::group> groups;
};


template<typename Texture>
class texture_animator {
public:
  using texture_type = Texture;

private:
  struct entry {
    const texture_atlas<texture_type>::sequence& sequence;
    uint duration, clock{0};
  };

public:
  texture_animator() = default;

  texture_animator(const texture_atlas<texture_type>& atlas, std::string_view first_sequence) :
    _atlas(&atlas) { enqueue_sequence(first_sequence, 0); }

public:
  void enqueue_sequence(std::string_view sequence, uint loops);
  void enqueue_sequence_frames(std::string_view sequence, uint frames);
  void soft_switch(std::string_view sequence, uint loops);
  void hard_switch(std::string_view sequence, uint loops);

public:
  void tick();
  texture_atlas<texture_type>::texture next_frame() const;

private:
  void reset_queue(bool hard);
  
private:
  const texture_atlas<texture_type>* _atlas;
  std::queue<entry> _sequences;
};

} // namespace ntf

#ifndef SHOGLE_ASSETS_ATLAS_INL
#include <shogle/assets/atlas.inl>
#endif
