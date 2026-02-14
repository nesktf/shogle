#pragma once

#include "./matrix4x4.hpp"

namespace shogle::math {

template<typename Func, typename T>
concept cam3d_view_func =
  requires(const Func f, const ::shogle::numvec<3, typename Func::value_type>& pos,
           const ::shogle::numvec<3, typename Func::value_type>& target,
           const ::shogle::numvec<3, typename Func::value_type>& up) {
    {
      std::invoke(f, pos, target, up)
    } -> std::same_as<::shogle::nummat<4, 4, typename Func::value_type>>;
    requires noexcept(std::invoke(f, pos, target, up));
    std::convertible_to<typename Func::value_type, T>;
  };

template<numeric_type T>
struct cam3d_lookat_rhs {
public:
  using value_type = T;

public:
  SHOGLE_MATH_DEF nummat<4, 4, T> operator()(const ::shogle::numvec<3, T>& pos,
                                             const ::shogle::numvec<3, T>& target,
                                             const ::shogle::numvec<3, T>& up) const noexcept {
    return ::shogle::math::lookat_rh(pos, target, up);
  }
};

template<numeric_type T>
struct cam3d_lookat_lhs {
public:
  using value_type = T;

public:
  SHOGLE_MATH_DEF nummat<4, 4, T> operator()(const ::shogle::numvec<3, T>& pos,
                                             const ::shogle::numvec<3, T>& target,
                                             const ::shogle::numvec<3, T>& up) const noexcept {
    return ::shogle::math::lookat_lh(pos, target, up);
  }
};

enum class cam3d_movement {
  forward = 0,
  backward,
  left,
  right,
  up,
  down,
};

template<numeric_type T, cam3d_view_func<T> ViewFunc = cam3d_lookat_rhs<T>>
struct camera3d_euler {
public:
  using value_type = T;
  using func_type = ViewFunc;

public:
  camera3d_euler() noexcept :
      pos(T(0), T(0), T(0)), world_up(T(0), T(1), T(0)), _front(T(0), T(0), T(1)),
      _up(T(0), T(1), T(0)), _right(T(-1), T(0), T(0)), _yaw(::shogle::math::rad(T(-90))),
      _pitch(T(0)), _mouse_sens(T(0.0025)) {}

public:
  void process_mouse(T xoff, T yoff, bool clamp_pitch = true) {
    yoff *= _mouse_sens;
    xoff *= _mouse_sens;

    _yaw += xoff;
    _pitch += yoff;

    if (clamp_pitch) {
      constexpr T pitch_limit = ::shogle::math::rad(T(89));
      _pitch = ::shogle::math::clamp(_pitch, -pitch_limit, pitch_limit);
    }
    _update_vecs();
  }

  void process_movement(cam3d_movement movement, T speed) {
    switch (movement) {
      case cam3d_movement::forward: {
        pos += _front * speed;
      } break;
      case cam3d_movement::backward: {
        pos -= _front * speed;
      } break;
      case cam3d_movement::left: {
        pos -= _right * speed;
      } break;
      case cam3d_movement::right: {
        pos += _right * speed;
      } break;
      case cam3d_movement::up: {
        pos += world_up * speed;
      } break;
      case cam3d_movement::down: {
        pos -= world_up * speed;
      } break;
    }
  }

public:
  nummat<4, 4, T> matrix() const noexcept { return get_func()(pos, pos + _front, _up); }

public:
  SHOGLE_MATH_DEF func_type& get_func() noexcept { return static_cast<ViewFunc&>(*this); }

  SHOGLE_MATH_DEF const func_type& get_func() const noexcept {
    return static_cast<const ViewFunc&>(*this);
  }

private:
  SHOGLE_MATH_DEF void _update_vecs() noexcept {
    const T cpitch = ::shogle::math::cos(_pitch);
    numvec<3, T> front;
    front.x = ::shogle::math::cos(_yaw) * cpitch;
    front.y = ::shogle::math::sin(_pitch);
    front.z = ::shogle::math::sin(_yaw) * cpitch;
    _right = ::shogle::math::normalize(::shogle::math::cross(front, world_up));
    _front = ::shogle::math::normalize(front);
    _up = ::shogle::math::normalize(::shogle::math::cross(_right, _front));
  }

public:
  ::shogle::numvec<3, T> pos;
  ::shogle::numvec<3, T> world_up;

private:
  ::shogle::numvec<3, T> _front;
  ::shogle::numvec<3, T> _up;
  ::shogle::numvec<3, T> _right;
  T _yaw, _pitch, _mouse_sens;
};

} // namespace shogle::math
