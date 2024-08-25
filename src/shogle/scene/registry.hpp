#pragma once

#include <shogle/shogle.hpp>
#include <shogle/core/sparse_set.hpp>

namespace ntf {

// Shitty and incomplete ECS, do not use

template<size_t max_entities>
class registry {
public:
  using entity_id = uint32_t;

  class entity {
  public:
    entity(entity_id id, registry* registry) :
      _id(id), _registry(registry) {}

  public:
    template<typename C>
    auto& add(C comp) {
      return _registry->add_component(this->_id, std::move(comp));
    }

    template<typename C>
    auto& get() {
      return _registry->get_component<C>(this->_id);
    }

    template<typename C>
    auto opt() {
      return _registry->opt_component<C>(this->_id);
    }

  public:
    entity_id id() const { return _id; }

  private:
    entity_id _id;
    registry* _registry;
    friend class registry;
  };

  template<typename derived_type, size_t max_components = max_entities>
  class component {
  protected:
    component() = default;

  public:
    static auto& get_set() {
      static ntf::sparse_set<derived_type, entity_id, max_components, max_entities> set;
      return set;
    }
  };

public:
  registry() = default;

public:
  entity create() {
    _entities.emplace_back(++curr_id, this);
    return _entities.back();
  };

  template<typename C>
  auto& get_component(entity_id id) {
    auto& set = C::get_set();
    assert(set.valid(id) && "Entity doesn't have component");
    return set.get(id);
  }

  template<typename C>
  auto opt_component(entity_id id) {
    auto& set = C::get_set();
    return set.opt(id);
  }

  template<typename C>
  auto& add_component(entity_id id, C comp) {
    auto& set = C::get_set();
    set.insert(id, std::move(comp));
    return set.get(id);
  }

  template<typename C, typename... Args>
  auto& emplace_component(entity_id id, Args&&... args) {
    auto& set = C::get_set();
    set.emplace(id, std::forward<Args>(args)...);
    return set.get(id);
  }

private:
  entity_id curr_id{};
  std::vector<entity> _entities;
};

} // namespace ntf
