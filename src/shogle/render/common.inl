#define SHOGLE_RENDER_COMMON_INL
#include <shogle/render/common.hpp>
#undef SHOGLE_RENDER_COMMON_INL

namespace ntf {

template<typename Shader>
uniform_tuple<Shader>::uniform_tuple(std::vector<entry> entries) {
  auto enumtosz = [](uniform_category type) -> size_t {
    switch (type) {
      case uniform_category::scalar:
        return sizeof(float);
      case uniform_category::iscalar:
        return sizeof(int);
      case uniform_category::vec2:
        return sizeof(vec2);
      case uniform_category::vec3:
        return sizeof(vec3);
      case uniform_category::vec4:
        return sizeof(vec4);
      case uniform_category::mat3:
        return sizeof(mat3);
      case uniform_category::mat4:
        return sizeof(mat4);
    }
    return 0; // shutup gcc
  };

  size_t data_size = 0;
  for (const auto& [uniform, type] : entries) {
    _uniforms.emplace(std::make_pair(uniform, std::make_pair(type, data_size)));
    data_size += enumtosz(type);
  }
  _data.reserve(data_size);
}

template<typename Shader>
void uniform_tuple<Shader>::clear() {
  _uniforms.clear();
  _data.clear();
}

template<typename Shader>
void uniform_tuple<Shader>::bind(const shader_type& shader) const {
  shader.use();
  for (const auto& [uniform, pair] : _uniforms) {
    const auto& [type, offset] = pair;
    const uint8_t* pos = &_data[0]+offset;
    switch (type) {
      case uniform_category::scalar: {
        float stored;
        std::memcpy(&stored, pos, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
      case uniform_category::iscalar: {
        int stored;
        std::memcpy(&stored, pos, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      };
      case uniform_category::vec2: {
        vec2 stored;
        std::memcpy(&stored, pos, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
      case uniform_category::vec3: {
        vec3 stored;
        std::memcpy(&stored, pos, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
      case uniform_category::vec4: {
        vec4 stored;
        std::memcpy(&stored, pos, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
      case uniform_category::mat3: {
        mat3 stored;
        std::memcpy(&stored, pos, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
      case uniform_category::mat4: {
        mat4 stored;
        std::memcpy(&stored, pos, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
    }
  }
}

template<typename Shader>
template<typename T>
requires(uniform_traits<T>::is_uniform)
bool uniform_tuple<Shader>::set_uniform(uniform_type uniform, T&& val) {
  if (_uniforms.find(uniform) == _uniforms.end()) {
    return false;
  }
  const auto& [type, offset] = _uniforms.at(uniform);

  if (type != uniform_traits<T>::enum_value) {
    return false;
  }
  std::memcpy(&val, &_data[0]+offset, uniform_traits<T>::size);

  return true;
}

} // namespace ntf
