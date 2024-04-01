#pragma once

#include "types.hpp"

#include <glm/mat4x4.hpp>

#include <concepts>

namespace ntf::shogle {

template<typename TDraw>
concept is_drawable = requires (TDraw drawable, TransformData data) {
  requires std::same_as<decltype(std::declval<TDraw>().model_m), glm::mat4>;
  { drawable.model_transform(data) } -> std::same_as<glm::mat4>;
};

template<typename TRes>
concept is_resource = requires (TRes res) {
  requires std::constructible_from<TRes, const typename TRes::data_t*>;
  requires std::constructible_from<typename TRes::data_t, std::string>;
  requires !std::copy_constructible<TRes> && std::move_constructible<TRes>;
  requires !std::copyable<TRes> && std::movable<TRes>;
};

template<typename T, typename... TRes>
concept same_as_defined = (... or std::same_as<T, TRes>);



} // namespace ntf::shogle
