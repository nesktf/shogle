#pragma once

#include "core/manipulator.hpp"
#include "sol/sol.hpp"
#include <memory>

namespace ntf::shogle {

class Scene {
public:
  Scene();

  void update(float delta_time);
  void draw(void) const;

// protected:
//   template <typename T>
//   void init_resources(const ResourceList& list, ResourceMap<T>& map) {
//     this->load_c += map.size();
//     ResourceLoader::instance().request_resources<typename T::data_t>(list, [this, &map](auto data, auto id) {
//       map.emplace(std::make_pair(id, T{std::move(data)}));
//       if (--this->load_c == 0) {
//         this->on_load();
//       }
//     });
//   }
//
// private:
  // size_t load_c;

public:
  std::vector<GameObject> objs;
  std::vector<std::unique_ptr<Manipulator>> obj_manip;
  std::vector<std::unique_ptr<Manipulator>> new_obj_manip;
};

} // namespace ntf::shogle
