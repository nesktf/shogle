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
  _data = new uint8_t[data_size];
}

template<typename Shader>
uniform_tuple<Shader>::~uniform_tuple() noexcept { clear(); }

template<typename Shader>
uniform_tuple<Shader>::uniform_tuple(uniform_tuple&& t) noexcept :
  _uniforms{std::move(t._uniforms)}, _data(std::move(_data)) { t._data = nullptr; }

template<typename Shader>
auto uniform_tuple<Shader>::operator=(uniform_tuple&& t) noexcept -> uniform_tuple& {
  clear();

  _uniforms = std::move(t._uniforms);
  _data = std::move(t._data);

  t._data = nullptr;

  return *this;
}

template<typename Shader>
void uniform_tuple<Shader>::clear() {
  _uniforms.clear();
  if (_data) {
    delete[] _data;
  }
}

template<typename Shader>
void uniform_tuple<Shader>::bind(const shader_type& shader) const {
  shader.use();
  for (const auto& [uniform, pair] : _uniforms) {
    const auto& [type, offset] = pair;
    switch (type) {
      case uniform_category::scalar: {
        float stored;
        std::memcpy(_data+offset, &stored, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
      case uniform_category::iscalar: {
        int stored;
        std::memcpy(_data+offset, &stored, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      };
      case uniform_category::vec2: {
        vec2 stored;
        std::memcpy(_data+offset, &stored, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
      case uniform_category::vec3: {
        vec3 stored;
        std::memcpy(_data+offset, &stored, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
      case uniform_category::vec4: {
        vec4 stored;
        std::memcpy(_data+offset, &stored, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
      case uniform_category::mat3: {
        mat3 stored;
        std::memcpy(_data+offset, &stored, sizeof(stored));
        shader.set_uniform(uniform, stored);
        break;
      }
      case uniform_category::mat4: {
        mat4 stored;
        std::memcpy(_data+offset, &stored, sizeof(stored));
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
  std::memcpy(_data+offset, &val, uniform_traits<T>::size);

  return true;
}

} // namespace ntf
