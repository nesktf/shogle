#pragma once

#include "./assets.hpp"
#include "./texture.hpp"

namespace ntf {

using atlas_handle = uint16;
constexpr atlas_handle atlas_tombstone = std::numeric_limits<atlas_handle>::max();

template<typename Alloc = std::allocator<uint8>>
class texture_atlas_data {
public:
  using allocator_type = Alloc;
  using texture_data_type = texture_data<rebind_alloc_t<uint8, Alloc>>;

  struct sprite_data {
    vec4 offset;
    uvec2 dim;

    vec2 aspect() const { return vec2{static_cast<float32>(dim.x)/static_cast<float32>(dim.y)}; }
    vec2 uv_coord(vec2 uv) const { return vec2{uv.x*offset.x+offset.z, uv.y*offset.y+offset.w}; }
  };

  struct sprite_group {
    template<typename GroupAlloc>
    sprite_group(const GroupAlloc& alloc) :
      name(alloc), entries(alloc) {}

    std::basic_string<char, std::char_traits<char>, rebind_alloc_t<char, Alloc>> name;
    std::vector<atlas_handle, rebind_alloc_t<atlas_handle, Alloc>> entries;
  };

  struct sprite_sequence {
    template<typename SeqAlloc>
    sprite_sequence(const SeqAlloc& alloc) :
      name(alloc), entries(alloc) {}

    std::basic_string<char, std::char_traits<char>, rebind_alloc_t<char, Alloc>> name;
    std::vector<atlas_handle, rebind_alloc_t<atlas_handle, Alloc>> entries;
  };
  
  using value_type = sprite_data;
  // using iterator =
  //   std::vector<sprite_data, rebind_alloc_t<sprite_data, Alloc>>::iterator;
  using const_iterator =
    std::vector<sprite_data, rebind_alloc_t<sprite_data, Alloc>>::const_iterator;

private:
  using json = nlohmann::json;

public:
  texture_atlas_data()
  noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
    _texture_path(Alloc{}), _sprites(Alloc{}), _sequences(Alloc{}), _groups(Alloc{}) {}

  explicit texture_atlas_data(const Alloc& alloc)
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _texture_path(alloc), _sprites(alloc), _sequences(alloc), _groups(alloc) {}

  explicit texture_atlas_data(std::string_view path)
  noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
    _texture_path(Alloc{}), _sprites(Alloc{}), _sequences(Alloc{}), _groups(Alloc{}) { load(path); }

  texture_atlas_data(std::string_view path, const Alloc& alloc) 
  noexcept(std::is_nothrow_copy_constructible_v<Alloc>) :
    _texture_path(alloc), _sprites(alloc), _sequences(alloc), _groups(alloc) { load(path); }

public:
  void load(std::string_view path) {
    // TODO: Make noexcept
    NTF_ASSERT(!has_data());

    auto tex_dir = file_dir(path);
    if (!tex_dir) {
      SHOGLE_LOG(error, "[ntf::texture_atlas_data] Invalid path \"{}\"", path);
      return;
    }

    std::ifstream f{path.data()};
    if (!f.is_open()) {
      SHOGLE_LOG(error, "[ntf::texture_atlas_data] Failed to open json \"{}\"", path);
      return;
    }

    json json_data = json::parse(f, nullptr, false);
    if (json_data.is_discarded()) {
      SHOGLE_LOG(error, "[ntf::texture_atlas_data] Failed to parse json file \"{}\"", path);
      return;
    }

    std::string tex_path = *tex_dir+"/"+json_data["file"].get<std::string>();
    auto tex_info = texture_info(tex_path);
    if (!tex_info) {
      SHOGLE_LOG(error, "[ntf::texture_atlas_data] Failed to open texture file \"{}\"", tex_path);
      return;
    }

    uint32 sprite_pos = 0;
    _preallocate_vectors(json_data["content"], path);
    for (const auto& content : json_data["content"]) {
      const std::string group_name = content["name"].get<std::string>();

      const uint32 group_size = _parse_offsets(tex_info->first, content["offset"]);
      _parse_sequences(sprite_pos, group_name, content["anim"]);

      _groups.emplace_back(_sprites.get_allocator());
      sprite_group& stored = _groups.back();

      stored.name.resize(group_name.size());
      std::memcpy(stored.name.data(), group_name.data(), group_name.size());

      stored.entries.reserve(group_size);
      for (uint32 i = 0; i < group_size; ++i) {
        stored.entries.emplace_back(i+sprite_pos);
      }

      sprite_pos += group_size;
    }
    _texture_path.resize(tex_path.size());
    std::memcpy(_texture_path.data(), tex_path.data(), tex_path.size());
  }

  void unload() noexcept {
    NTF_ASSERT(has_data());
    _sprites.clear();
    _sequences.clear();
    _groups.clear();
  }

public:
  const sprite_data& sprite(atlas_handle pos) const {
    NTF_ASSERT(pos < _sprites.size());
    return _sprites[pos];
  }
  const sprite_sequence& sequence(atlas_handle pos) const {
    NTF_ASSERT(pos < _sequences.size());
    return _sequences[pos];
  }
  const sprite_group& group(atlas_handle pos) const {
    NTF_ASSERT(pos < _groups.size());
    return _groups[pos];
  }
  atlas_handle find_sequence(std::string_view name) const {
    const auto it = std::find_if(_sequences.begin(), _sequences.end(), [&](const auto& seq) {
      return seq.name == name;
    });
    if (it == _sequences.end()) {
      return atlas_tombstone;
    }
    return static_cast<atlas_handle>(std::distance(_sequences.begin(), it));
  }
  atlas_handle find_group(std::string_view name) const {
    const auto it = std::find_if(_groups.begin(), _groups.end(), [&](const auto& group) {
      return group.name == name;
    });
    if (it == _groups.end()) {
      return atlas_tombstone;
    }
    return static_cast<atlas_handle>(std::distance(_groups.begin(), it));
  }

  uint32 sprite_count() const { return static_cast<uint32>(_sprites.size()); }
  uint32 group_count() const { return static_cast<uint32>(_groups.size()); }
  uint32 sequence_count() const { return static_cast<uint32>(_sequences.size()); }

  std::string_view path() const { return _texture_path; }

  [[nodiscard]] bool has_data() const { return _sprites.size() > 0; }
  explicit operator bool() const { return has_data(); }

  const sprite_data& operator[](atlas_handle pos) const { return sprite(pos); }
  const sprite_data& at(atlas_handle pos) const { return sprite(pos); }

  // iterator begin() { return _sprites.begin(); }
  const_iterator begin() const { return _sprites.begin(); }
  const_iterator cbegin() const { return _sprites.cbegin(); }

  // iterator end() { return _sprites.end(); }
  const_iterator end() const { return _sprites.end(); }
  const_iterator cend() const { return _sprites.cend(); }

private:
  void _preallocate_vectors(const json& content_entry, [[maybe_unused]] std::string_view path) {
    uint32 sprite_count = 0;
    uint32 sequence_count = 0;
    for (const auto& content : content_entry) {
      sprite_count += content["offset"]["count"].get<uint32>();
      sequence_count += content["anim"].size();
    }
    SHOGLE_LOG(verbose,
               "[ntf::texture_atlas_data] Found {} sprites, {} groups, {} sequences for \"{}\"",
               sprite_count, content_entry.size(), sequence_count, path);
    _sprites.reserve(sprite_count);
    _groups.reserve(content_entry.size());
    _sequences.reserve(sequence_count);
  }

  uint32 _parse_offsets(uvec2 tex_dim, const json& entry) {
    const uint32 count = entry["count"].get<uint32>();

    const uint32 x0 = entry["x0"].get<uint32>();
    const uint32 y0 = entry["y0"].get<uint32>();
    const uint32 dx = entry["dx"].get<uint32>();
    const uint32 dy = entry["dy"].get<uint32>();

    const uint32 cols = entry["cols"].get<uint32>();
    const float32 row_ratio = static_cast<float32>(count)/static_cast<float32>(cols);
    const uint32 rows = static_cast<uint32>(std::ceil(row_ratio));

    const uvec2 dim{dx/cols, dy/rows}; // all the sprites in the group have the same size
    const vec2 linear_offset = static_cast<vec2>(dim)/static_cast<vec2>(tex_dim);

    for (uint32 i = 0; i < count; ++i) {
      const uint32 row = i / cols;
      const uint32 col = i % cols;

      const vec2 frac_a {(x0*cols) + (col*dx), (y0*rows) + (row*dy)};
      const vec2 frac_b {tex_dim.x*cols, tex_dim.y*rows};
      const vec4 offset{linear_offset, vec2{frac_a.x/frac_b.x, frac_a.y/frac_b.y}};

      _sprites.emplace_back(offset, dim);
    }

    return count;
  }

  void _parse_sequences(uint32 sprite_pos, const std::string& group_name, const json& entry) {
    for (const auto& seq_entry : entry) {
      const auto& seq = seq_entry["sequence"];
      const uint32 delay = seq_entry["delay"].get<uint32>();
      const uint32 total_frames = seq.size()*delay;
      const std::string seq_name = group_name+"/"+seq_entry["name"].get<std::string>();

      _sequences.emplace_back(_sprites.get_allocator());
      sprite_sequence& stored = _sequences.back();
      stored.name.resize(seq_name.size());
      std::memcpy(stored.name.data(), seq_name.data(), seq_name.size());

      stored.entries.resize(total_frames, 0);
      for (uint32 i = 0, curr_frame = 0; i < total_frames; i += delay, ++curr_frame) {
        uint32 index = seq[curr_frame];
        for (uint32 j = 0; j < delay; ++j) {
          stored.entries[j+i] = sprite_pos+index;
        }
      }
    }
  }

public:
  static sprite_data default_sprite(uvec2 dim) {
    return sprite_data{vec4{1.f, 1.f, 0.f, 0.f}, dim};
  }

private:
  std::basic_string<char, std::char_traits<char>, rebind_alloc_t<char, Alloc>> _texture_path;
  std::vector<sprite_data, rebind_alloc_t<sprite_data, Alloc>> _sprites;
  std::vector<sprite_sequence, rebind_alloc_t<sprite_sequence, Alloc>> _sequences;
  std::vector<sprite_group, rebind_alloc_t<sprite_group, Alloc>> _groups;
};

// template<typename Texture, typename AtlasPtr = texture_atlas<Texture>*>
// class texture_animator {
// public:
//   using texture_type = Texture;
//   using atlas_type = texture_atlas<Texture>;
//
// private:
//   struct entry {
//     atlas_sequence sequence;
//     uint duration, clock{0};
//   };
//
// public:
//   texture_animator() = default;
//
//   texture_animator(const AtlasPtr atlas, atlas_sequence first_sequence) :
//     _atlas(atlas) { enqueue_sequence(first_sequence, 0); }
//
// public:
//   void enqueue_sequence(atlas_sequence sequence, uint loops) {
//     const auto& seq = _atlas->sequence_at(sequence);
//     _sequences.push(entry{
//       .sequence=sequence,
//       .duration = loops*static_cast<uint>(seq.size())
//     });
//   }
//   void enqueue_sequence_frames(atlas_sequence sequence, uint frames) {
//     enqueue_sequence(sequence, 0);
//     auto& enqueued = _sequences.back();
//     enqueued.duration = frames;
//   }
//   void soft_switch(atlas_sequence sequence, uint loops) {
//     reset_queue(false);
//     enqueue_sequence(sequence, loops);
//   }
//   void hard_switch(atlas_sequence sequence, uint loops) {
//     reset_queue(true);
//     enqueue_sequence(sequence, loops);
//   }
//
// public:
//   void tick() {
//     auto& next = _sequences.front();
//     const auto& seq = _atlas->sequence_at(next.sequence);
//     uint& clock = next.clock;
//     clock++;
//     if (clock >= next.duration && _sequences.size() > 1 && clock%seq.size() == 0) {
//       _sequences.pop();
//     }
//   }
//   atlas_texture frame() const {
//     const auto& next = _sequences.front();
//     const auto& seq = _atlas->sequence_at(next.sequence);
//     return seq[next.clock%seq.size()];
//   }
//   AtlasPtr atlas() const { return _atlas; }
//
//   bool valid() const { return _atlas->valid(); }
//
// private:
//   void reset_queue(bool hard);
//   
// private:
//   AtlasPtr _atlas;
//   std::queue<entry> _sequences;
// };

} // namespace ntf
