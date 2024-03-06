#pragma once

#include "core/game_object.hpp"
#include "resources/model_data.hpp"
#include "resources/texture_data.hpp"
#include "resources/shader_data.hpp"
#include "resources/loader.hpp"
#include "core/model.hpp"
#include "core/texture.hpp"
#include "core/shader.hpp"
#include <unordered_map>
#include <memory>

namespace ntf::shogle {

class Scene {
public:
  class State {
  public:
    virtual ~State() = default;

    virtual void on_update(float delta_time, Scene* scene) = 0;
    virtual void on_draw(Scene* scene) = 0;
  };

public:
  virtual ~Scene();
  virtual void on_load() = 0;

public:
  void update(float delta_time);
  void draw(void) const;
  void set_state(Scene::State* new_state);

protected:
  template <typename T>
  void init_resources(const ResourceList& list, ResourceMap<T>& map) {
    this->load_c += map.size();
    ResourceLoader::instance().request_resources<typename T::data_t>(list, [this, &map](auto data, auto id) {
      map.emplace(std::make_pair(id, T{std::move(data)}));
      if (--this->load_c == 0) {
        this->on_load();
      }
    });
  }

protected:
  Scene::State* prev_state;
  Scene::State* state;

private:
  size_t load_c;
};

} // namespace ntf::shogle
