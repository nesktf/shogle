#pragma once

#include "task/task.hpp"

#include "log.hpp"
#include "types.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace ntf::shogle {

using id_t = std::string;

template<typename TObj>
using ObjMap = std::unordered_map<id_t, std::unique_ptr<TObj>>;

template<typename TObj>
inline std::pair<id_t,std::unique_ptr<TObj>> make_pair_ptr(id_t id, TObj* obj) {
  return std::make_pair(id, std::unique_ptr<TObj>{obj});
}

template<typename TDrawable>
class GameObject {
public: 
  template<typename... Args>
  GameObject(Args&&... args) :
    obj(TDrawable{std::forward<Args>(args)...}) {}

  ~GameObject() = default;

  GameObject(GameObject&&) = default;
  GameObject& operator=(GameObject&&) = default;

  GameObject(const GameObject&) = default;
  GameObject& operator=(const GameObject&) = default;

public:
  inline void set_transform(TransformData transf) {
    this->transform = transf;
    obj.model_m = TDrawable::model_transform(transf);
  }

  inline TransformData get_transform(void) {
    return this->transform;
  }

  inline void draw(void) { if (enable) obj.draw(); }

public:
  TDrawable obj;
  TransformData transform {};
  bool enable {true};
};

} // namespace ntf::shogle
