#pragma once

#include "./matrix4x4.hpp"
#include "./vector2.hpp"

namespace shogle::math {

template<typename Transf>
struct cached_transform : private Transf {
public:
  SHOGLE_MATH_DEF cached_transform() noexcept : Transf(), _mat(), _dirty(true) {}

public:
  SHOGLE_MATH_DEF const nummat<4, 4, Transf>& matrix() noexcept {
    if (_dirty) {
      _mat = Transf::matrix();
    }
    return _mat;
  }

public:
  SHOGLE_MATH_DEF typename Transf::func_type& get_func() noexcept { return Transf::get_func(); }

  SHOGLE_MATH_DEF const typename Transf::func_type& get_func() const noexcept {
    return Transf::get_func();
  }

public:
  ::shogle::nummat<4, 4, Transf> _mat;
  bool _dirty;
};

template<typename Transf, typename T>
concept transform2d_func =
  requires(const Transf t, const ::shogle::numvec<2, typename Transf::value_type>& pos,
           const ::shogle::numvec<2, typename Transf::value_type>& scale,
           const ::shogle::numvec<2, typename Transf::value_type>& pivot,
           const ::shogle::numvec<2, typename Transf::value_type>& offset,
           typename Transf::value_type rot) {
    {
      std::invoke(t, pos, scale, pivot, offset, rot)
    } -> std::same_as<::shogle::nummat<4, 4, typename Transf::value_type>>;
    requires noexcept(std::invoke(t, pos, scale, pivot, offset, rot));
    requires std::convertible_to<typename Transf::value_type, T>;
  };

template<numeric_type T>
struct trs_transform2d_roll {
public:
  using value_type = T;

public:
  SHOGLE_MATH_DEF nummat<4, 4, T> operator()(const ::shogle::numvec<2, T>& pos,
                                             const ::shogle::numvec<2, T>& scale,
                                             const ::shogle::numvec<2, T>& pivot,
                                             const ::shogle::numvec<2, T>& offset,
                                             T roll) const noexcept {
    constexpr ::shogle::numvec<3, T> rolldir(T(0), T(0), T(1));
    const numvec<3, T> scale3(scale.x, scale.y, T(1));
    const numvec<3, T> tr1(pos.x + pivot.x, pos.y + pivot.y, T(0));
    const numvec<3, T> tr2(offset.x - pivot.x, offset.y - pivot.y, T(0));
    const numvec<3, T> tr3(-offset.x, -offset.y, T(0));

    nummat<4, 4, T> out(T(1));
    out = ::shogle::math::translate(out, tr1);
    out = ::shogle::math::rotate(out, roll, rolldir);
    out = ::shogle::math::translate(out, tr2);
    out = ::shogle::math::scale(out, scale3);
    out = ::shogle::math::translate(out, tr3);
    return out;
  }
};

template<typename Transf, typename T>
concept transform_quat_func =
  requires(const Transf t, const ::shogle::numvec<3, typename Transf::value_type>& pos,
           const ::shogle::numvec<3, typename Transf::value_type>& scale,
           const ::shogle::numvec<3, typename Transf::value_type>& pivot,
           const ::shogle::numvec<3, typename Transf::value_type>& offset,
           const ::shogle::numquat<typename Transf::value_type>& rot) {
    {
      std::invoke(t, pos, scale, pivot, offset, rot)
    } -> std::same_as<::shogle::nummat<4, 4, typename Transf::value_type>>;
    requires noexcept(std::invoke(t, pos, scale, pivot, offset, rot));
    requires std::convertible_to<typename Transf::value_type, T>;
  };

template<numeric_type T>
struct trs_transform3d_quat {
public:
  using value_type = T;

public:
  SHOGLE_MATH_DEF nummat<4, 4, T> operator()(const ::shogle::numvec<3, T>& pos,
                                             const ::shogle::numvec<3, T>& scale,
                                             const ::shogle::numvec<3, T>& pivot,
                                             const ::shogle::numvec<3, T>& offset,
                                             const ::shogle::numquat<T>& rot) const noexcept {
    const nummat<4, 4, T> rot_mat = ::shogle::math::to_mat4(rot);

    nummat<4, 4, T> out(T(1));
    out = ::shogle::math::translate(out, pos + pivot);
    out *= rot_mat;
    out = ::shogle::math::translate(out, offset - pivot);
    out = ::shogle::math::scale(out, scale);
    out = ::shogle::math::translate(out, -offset);
    return out;
  }
};

template<typename Transf, typename T>
concept transform_euler_func =
  requires(const Transf t, const ::shogle::numvec<3, typename Transf::value_type>& pos,
           const ::shogle::numvec<3, typename Transf::value_type>& scale,
           const ::shogle::numvec<3, typename Transf::value_type>& pivot,
           const ::shogle::numvec<3, typename Transf::value_type>& offset,
           const ::shogle::numvec<3, typename Transf::value_type>& rot) {
    {
      std::invoke(t, pos, scale, pivot, offset, rot)
    } -> std::same_as<::shogle::nummat<4, 4, typename Transf::value_type>>;
    requires noexcept(std::invoke(t, pos, scale, pivot, offset, rot));
    requires std::convertible_to<typename Transf::value_type, T>;
  };

template<numeric_type T>
struct trs_transform3d_euler {
public:
  using value_type = T;

public:
  SHOGLE_MATH_DEF nummat<4, 4, T> operator()(const ::shogle::numvec<3, T>& pos,
                                             const ::shogle::numvec<3, T>& scale,
                                             const ::shogle::numvec<3, T>& pivot,
                                             const ::shogle::numvec<3, T>& offset,
                                             const ::shogle::numvec<3, T>& euler) const noexcept {
    constexpr ::shogle::numvec<3, T> pitchdir(T(1), T(0), T(0));
    constexpr ::shogle::numvec<3, T> yawdir(T(0), T(1), T(0));
    constexpr ::shogle::numvec<3, T> rolldir(T(0), T(0), T(1));

    nummat<4, 4, T> out(T(1));
    out = ::shogle::math::translate(out, pos + pivot);
    out = ::shogle::math::rotate(out, euler.z, rolldir);
    out = ::shogle::math::rotate(out, euler.y, yawdir);
    out = ::shogle::math::rotate(out, euler.x, pitchdir);
    out = ::shogle::math::translate(out, offset - pivot);
    out = ::shogle::math::scale(out, scale);
    return out;
  }
};

template<numeric_type T, transform2d_func<T> TransformFunc = trs_transform2d_roll<T>>
struct transform2d : private TransformFunc {
public:
  using value_type = T;
  using func_type = TransformFunc;

public:
  SHOGLE_MATH_DEF transform2d() noexcept :
      TransformFunc(), pos(T(0), T(0)), scale(T(1), T(1)), pivot(T(0), T(0)), offset(T(0), T(0)),
      rot(T(0)) {}

public:
  SHOGLE_MATH_DEF nummat<4, 4, T> matrix() const noexcept {
    return get_func()(pos, scale, pivot, offset, rot);
  }

public:
  SHOGLE_MATH_DEF func_type& get_func() noexcept { return static_cast<TransformFunc&>(*this); }

  SHOGLE_MATH_DEF const func_type& get_func() const noexcept {
    return static_cast<const TransformFunc&>(*this);
  }

public:
  ::shogle::numvec<2, T> pos;
  ::shogle::numvec<2, T> scale;
  ::shogle::numvec<2, T> pivot;
  ::shogle::numvec<2, T> offset;
  T rot;
};

template<typename T, typename TransformFunc = trs_transform2d_roll<T>>
using cached_transform2d = ::shogle::math::cached_transform<transform2d<T, TransformFunc>>;

template<numeric_type T, transform_quat_func<T> TransformFunc = trs_transform3d_quat<T>>
struct transform3d : private TransformFunc {
public:
  using value_type = T;
  using func_type = TransformFunc;

public:
  SHOGLE_MATH_DEF transform3d() noexcept :
      TransformFunc(), rot(T(1), T(0), T(0), T(0)), pos(T(0), T(0), T(0)), scale(T(1), T(1), T(1)),
      pivot(T(0), T(0), T(0)), offset(T(0), T(0), T(0)) {}

public:
  SHOGLE_MATH_DEF nummat<4, 4, T> matrix() const noexcept {
    return get_func()(pos, scale, pivot, offset, rot);
  }

public:
  SHOGLE_MATH_DEF func_type& get_func() noexcept { return static_cast<TransformFunc&>(*this); }

  SHOGLE_MATH_DEF const func_type& get_func() const noexcept {
    return static_cast<const TransformFunc&>(*this);
  }

public:
  ::shogle::numquat<T> rot;
  ::shogle::numvec<3, T> pos;
  ::shogle::numvec<3, T> scale;
  ::shogle::numvec<3, T> pivot;
  ::shogle::numvec<3, T> offset;
};

template<typename T, typename TransformFunc = trs_transform3d_quat<T>>
using cached_transform3d = ::shogle::math::cached_transform<transform3d<T, TransformFunc>>;

template<numeric_type T, transform_euler_func<T> TransformFunc = trs_transform3d_euler<T>>
struct transform3d_euler : private TransformFunc {
public:
  using value_type = T;
  using func_type = TransformFunc;

public:
  SHOGLE_MATH_DEF transform3d_euler() noexcept :
      TransformFunc(), pos(T(0), T(0), T(0)), scale(T(1), T(1), T(1)), pivot(T(0), T(0), T(0)),
      offset(T(0), T(0), T(0)), rot(T(0), T(0), T(0)) {}

public:
  SHOGLE_MATH_DEF nummat<4, 4, T> matrix() const noexcept {
    return get_func()(pos, scale, pivot, offset, rot);
  }

public:
  SHOGLE_MATH_DEF func_type& get_func() noexcept { return static_cast<TransformFunc&>(*this); }

  SHOGLE_MATH_DEF const func_type& get_func() const noexcept {
    return static_cast<const TransformFunc&>(*this);
  }

public:
  ::shogle::numvec<3, T> pos;
  ::shogle::numvec<3, T> scale;
  ::shogle::numvec<3, T> pivot;
  ::shogle::numvec<3, T> offset;
  ::shogle::numvec<3, T> rot;
};

template<typename T, typename TransformFunc = trs_transform3d_euler<T>>
using cached_transform3d_euler =
  ::shogle::math::cached_transform<transform3d_euler<T, TransformFunc>>;

} // namespace shogle::math
