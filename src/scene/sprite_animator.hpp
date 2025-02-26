#pragma once

#include "../assets/atlas.hpp"

namespace ntf {

template<typename Texture, typename AtlasPtr = texture_atlas<Texture>*>
class texture_animator {
public:
  using texture_type = Texture;
  using atlas_type = texture_atlas<Texture>;

private:
  struct entry {
    atlas_sequence sequence;
    uint duration, clock{0};
  };

public:
  texture_animator() = default;

  texture_animator(const AtlasPtr atlas, atlas_sequence first_sequence) :
    _atlas(atlas) { enqueue_sequence(first_sequence, 0); }

public:
  void enqueue_sequence(atlas_sequence sequence, uint loops) {
    const auto& seq = _atlas->sequence_at(sequence);
    _sequences.push(entry{
      .sequence=sequence,
      .duration = loops*static_cast<uint>(seq.size())
    });
  }
  void enqueue_sequence_frames(atlas_sequence sequence, uint frames) {
    enqueue_sequence(sequence, 0);
    auto& enqueued = _sequences.back();
    enqueued.duration = frames;
  }
  void soft_switch(atlas_sequence sequence, uint loops) {
    reset_queue(false);
    enqueue_sequence(sequence, loops);
  }
  void hard_switch(atlas_sequence sequence, uint loops) {
    reset_queue(true);
    enqueue_sequence(sequence, loops);
  }

public:
  void tick() {
    auto& next = _sequences.front();
    const auto& seq = _atlas->sequence_at(next.sequence);
    uint& clock = next.clock;
    clock++;
    if (clock >= next.duration && _sequences.size() > 1 && clock%seq.size() == 0) {
      _sequences.pop();
    }
  }
  atlas_texture frame() const {
    const auto& next = _sequences.front();
    const auto& seq = _atlas->sequence_at(next.sequence);
    return seq[next.clock%seq.size()];
  }
  AtlasPtr atlas() const { return _atlas; }

  bool valid() const { return _atlas->valid(); }

private:
  void reset_queue(bool hard);
  
private:
  AtlasPtr _atlas;
  std::queue<entry> _sequences;
};

} // namespace ntf
