#pragma once

#include "../math/matrix.hpp"
#include "../math/quaternion.hpp"
#include "../math/complex.hpp"

#define SHOGLE_TRANSF_DEF_SETTER(_signature, ...) \
Derived& _signature & \
noexcept(noexcept(static_cast<Derived&>(*this)._mark_dirty())) { \
  __VA_ARGS__ \
  static_cast<Derived&>(*this)._mark_dirty(); \
  return static_cast<Derived&>(*this); \
} \
Derived&& _signature && \
noexcept(noexcept(static_cast<Derived&>(*this)._mark_dirty())) { \
  __VA_ARGS__ \
  static_cast<Derived&>(*this)._mark_dirty(); \
  return static_cast<Derived&&>(*this); \
}

namespace ntf {

namespace impl {

template<typename T, typename Derived, uint32 dim>
class transform_dim;

template<typename T, typename Derived>
class transform_dim<T, Derived, 3> {
public:
  transform_dim(const vec<3, T>& pos    = {T{0}, T{0}, T{0}},
                const vec<3, T>& scale  = {T{1}, T{1}, T{1}},
                const vec<3, T>& pivot  = {T{0}, T{0}, T{0}},
                const vec<3, T>& offset = {T{0}, T{0}, T{0}}) noexcept :
    _pos{pos}, _scale{scale}, _pivot{pivot}, _offset{offset} {}

public:
  SHOGLE_TRANSF_DEF_SETTER(pos(const vec<3, T>& pos),
    _pos = pos;
  );
  SHOGLE_TRANSF_DEF_SETTER(pos(const T& x, const T& y, const T& z),
    _pos.x = x;
    _pos.y = y;
    _pos.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(pos(const vec<2, T>& pos_xy, const T& z),
    _pos.x = pos_xy.x;
    _pos.y = pos_xy.y;
    _pos.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(pos(const T& x, const vec<2, T>& pos_yz),
    _pos.x = x;
    _pos.y = pos_yz.x;
    _pos.z = pos_yz.y;
  );
  SHOGLE_TRANSF_DEF_SETTER(pos(const complex<T>& pos_xy, const T& z),
    _pos.x = pos_xy.real();
    _pos.y = pos_xy.imag();
    _pos.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(pos(const T& x, const complex<T>& pos_yz),
    _pos.x = x;
    _pos.y = pos_yz.real();
    _pos.z = pos_yz.imag();
  );
  SHOGLE_TRANSF_DEF_SETTER(pos_x(const T& x),
    _pos.x = x;
  );
  SHOGLE_TRANSF_DEF_SETTER(pos_y(const T& y),
    _pos.y = y;
  );
  SHOGLE_TRANSF_DEF_SETTER(pos_z(const T& z),
    _pos.z = z;
  );

  SHOGLE_TRANSF_DEF_SETTER(scale(const vec<3, T>& scale),
    _scale = scale;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale(const T& x, const T& y, const T& z),
    _scale.x = x;
    _scale.y = y;
    _scale.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale(const vec<2, T>& scale_xy, const T& z),
    _scale.x = scale_xy.x;
    _scale.y = scale_xy.y;
    _scale.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale(const T& x, const vec<2, T>& scale_yz),
    _scale.x = x;
    _scale.y = scale_yz.x;
    _scale.z = scale_yz.y;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale(const complex<T>& scale_xy, const T& z),
    _scale.x = scale_xy.real();
    _scale.y = scale_xy.imag();
    _scale.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale(const T& x, const complex<T>& scale_yz),
    _scale.x = x;
    _scale.y = scale_yz.real();
    _scale.z = scale_yz.imag();
  );
  SHOGLE_TRANSF_DEF_SETTER(scale(const T& scale),
    _scale.x = scale;
    _scale.y = scale;
    _scale.z = scale;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale_x(const T& x),
    _scale.x = x;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale_y(const T& y),
    _scale.y = y;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale_z(const T& z),
    _scale.z = z;
  );

  SHOGLE_TRANSF_DEF_SETTER(pivot(const vec<3, T>& pivot),
    _pivot = pivot;
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot(const T& x, const T& y, const T& z),
    _pivot.x = x;
    _pivot.y = y;
    _pivot.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot(const vec<2, T>& pivot_xy, const T& z),
    _pivot.x = pivot_xy.x;
    _pivot.y = pivot_xy.y;
    _pivot.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot(const T& x, const vec<2, T>& pivot_yz),
    _pivot.x = x;
    _pivot.y = pivot_yz.x;
    _pivot.z = pivot_yz.y;
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot(const complex<T>& pivot_xy, const T& z),
    _pivot.x = pivot_xy.real();
    _pivot.y = pivot_xy.imag();
    _pivot.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot(const T& x, const complex<T>& pivot_yz),
    _pivot.x = x;
    _pivot.y = pivot_yz.real();
    _pivot.z = pivot_yz.imag();
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot_x(const T& x),
    _pivot.x = x;
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot_y(const T& y),
    _pivot.y = y;
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot_z(const T& z),
    _pivot.z = z;
  );

  SHOGLE_TRANSF_DEF_SETTER(offset(const vec<3, T>& offset),
    _offset = offset;
  );
  SHOGLE_TRANSF_DEF_SETTER(offset(const T& x, const T& y, const T& z),
    _offset.x = x;
    _offset.y = y;
    _offset.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(offset(const vec<2, T>& offset_xy, const T& z),
    _offset.x = offset_xy.x;
    _offset.y = offset_xy.y;
    _offset.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(offset(const T& x, const vec<2, T>& offset_yz),
    _offset.x = x;
    _offset.y = offset_yz.x;
    _offset.z = offset_yz.y;
  );
  SHOGLE_TRANSF_DEF_SETTER(offset(const complex<T>& offset_xy, const T& z),
    _offset.x = offset_xy.real();
    _offset.y = offset_xy.imag();
    _offset.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(offset(const T& x, const complex<T>& offset_yz),
    _offset.x = x;
    _offset.y = offset_yz.real();
    _offset.z = offset_yz.imag();
  );
  SHOGLE_TRANSF_DEF_SETTER(offset_x(const T& x),
    _offset.x = x;
  );
  SHOGLE_TRANSF_DEF_SETTER(offset_y(const T& y),
    _offset.y = y;
  );
  SHOGLE_TRANSF_DEF_SETTER(offset_z(const T& z),
    _offset.z = z;
  );

public:
  [[nodiscard]] vec<3, T> pos() const noexcept { return _pos; }
  [[nodiscard]] T pos_x() const noexcept { return _pos.x; }
  [[nodiscard]] T pos_y() const noexcept { return _pos.y; }
  [[nodiscard]] T pos_z() const noexcept { return _pos.z; }

  [[nodiscard]] vec<3, T> scale() const noexcept { return _scale; }
  [[nodiscard]] T scale_x() const noexcept { return _scale.x; }
  [[nodiscard]] T scale_y() const noexcept { return _scale.y;}
  [[nodiscard]] T scale_z() const noexcept { return _scale.z; }

  [[nodiscard]] vec<3, T> pivot() const noexcept { return _pivot; }
  [[nodiscard]] T pivot_x() const noexcept { return _pivot.x; }
  [[nodiscard]] T pivot_y() const noexcept { return _pivot.y; }
  [[nodiscard]] T pivot_z() const noexcept { return _pivot.z; }

  [[nodiscard]] vec<3, T> offset() const noexcept { return _offset; }
  [[nodiscard]] T offset_x() const noexcept { return _offset.x; }
  [[nodiscard]] T offset_y() const noexcept { return _offset.y; }
  [[nodiscard]] T offset_z() const noexcept { return _offset.y; }

protected:
  vec<3, T> _pos;
  vec<3, T> _scale;
  vec<3, T> _pivot;
  vec<3, T> _offset;
};

template<typename T, typename Derived>
class transform_dim<T, Derived, 2> {
public:
  transform_dim(const vec<2, T>& pos    = {T{0}, T{0}},
                const vec<2, T>& scale  = {T{1}, T{1}},
                const vec<2, T>& pivot  = {T{0}, T{0}},
                const vec<2, T>& offset = {T{0}, T{0}}) noexcept :
    _pos{pos}, _scale{scale}, _pivot{pivot}, _offset{offset} {}

public:
  SHOGLE_TRANSF_DEF_SETTER(pos(const vec<2, T>& pos),
    _pos = pos;
  );
  SHOGLE_TRANSF_DEF_SETTER(pos(const complex<T>& pos),
    _pos.x = pos.real();
    _pos.y = pos.imag();
  );
  SHOGLE_TRANSF_DEF_SETTER(pos(const T& x, const T& y),
    _pos.x = x;
    _pos.y = y;
  );
  SHOGLE_TRANSF_DEF_SETTER(pos_x(const T& x),
    _pos.x = x;
  );
  SHOGLE_TRANSF_DEF_SETTER(pos_y(const T& y),
    _pos.y = y;
  );

  SHOGLE_TRANSF_DEF_SETTER(scale(const vec<2, T>& scale),
    _scale = scale;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale(const complex<T>& scale),
    _scale.x = scale.real();
    _scale.y = scale.imag();
  );
  SHOGLE_TRANSF_DEF_SETTER(scale(const T& x, const T& y),
    _scale.x = x;
    _scale.y = y;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale(const T& scale),
    _scale.x = scale;
    _scale.y = scale;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale_x(const T& x),
    _scale.x = x;
  );
  SHOGLE_TRANSF_DEF_SETTER(scale_y(const T& y),
    _scale.y = y;
  );

  SHOGLE_TRANSF_DEF_SETTER(pivot(const vec<2, T>& pivot),
    _pivot = pivot;
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot(const complex<T>& pivot),
    _pivot.x = pivot.real();
    _pivot.y = pivot.imag();
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot(const T& x, const T& y),
    _pivot.x = x;
    _pivot.y = y;
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot_x(const T& x),
    _pivot.x = x;
  );
  SHOGLE_TRANSF_DEF_SETTER(pivot_y(const T& y),
    _pivot.y = y;
  );

  SHOGLE_TRANSF_DEF_SETTER(offset(const vec<2, T>& offset),
    _offset = offset;
  );
  SHOGLE_TRANSF_DEF_SETTER(offset(const complex<T>& offset),
    _offset.x = offset.real();
    _offset.y = offset.imag();
  );
  SHOGLE_TRANSF_DEF_SETTER(offset(const T& x, const T& y),
    _offset.x = x;
    _offset.y = y;
  );
  SHOGLE_TRANSF_DEF_SETTER(offset_x(const T& x),
    _offset.x = x;
  );
  SHOGLE_TRANSF_DEF_SETTER(offset_y(const T& y),
    _offset.y = y;
  );

public:
  [[nodiscard]] vec<2, T> pos() const noexcept { return _pos; }
  [[nodiscard]] complex<T> cpos() const noexcept { return complex<T>{_pos.x, _pos.y}; }
  [[nodiscard]] T pos_x() const noexcept { return _pos.x; }
  [[nodiscard]] T pos_y() const noexcept { return _pos.y; }

  [[nodiscard]] vec<2, T> scale() const noexcept { return _scale; }
  [[nodiscard]] complex<T> cscale() const noexcept { return complex<T>{_scale.x, _scale.y}; }
  [[nodiscard]] T scale_x() const noexcept { return _scale.x; }
  [[nodiscard]] T scale_y() const noexcept { return _scale.y;}

  [[nodiscard]] vec<2, T> pivot() const noexcept { return _pivot; }
  [[nodiscard]] complex<T> cpivot() const noexcept { return complex<T>{_pivot.x, _pivot.y}; }
  [[nodiscard]] T pivot_x() const noexcept { return _pivot.x; }
  [[nodiscard]] T pivot_y() const noexcept { return _pivot.y; }

  [[nodiscard]] vec<2, T> offset() const noexcept { return _offset; }
  [[nodiscard]] complex<T> coffset() const noexcept { return complex<T>{_offset.x, _offset.y}; }
  [[nodiscard]] T offset_x() const noexcept { return _offset.x; }
  [[nodiscard]] T offset_y() const noexcept { return _offset.y; }

protected:
  vec<2, T> _pos;
  vec<2, T> _scale;
  vec<2, T> _pivot;
  vec<2, T> _offset;
};

template<typename T, typename Derived, bool use_euler>
class transform_rot {
public:
  explicit transform_rot(const vec<3, T>& rot = {T{0}, T{0}, T{0}}) noexcept :
    _rot{rot} {}
  explicit transform_rot(const qua<T>& rot) noexcept :
    _rot{eulerquat(rot)} {}

public:
  SHOGLE_TRANSF_DEF_SETTER(rot(const vec<3, T>& rot),
    _rot = rot;
  );
  SHOGLE_TRANSF_DEF_SETTER(rot(const qua<T>& rot),
    _rot = eulerquat(rot);
  );
  SHOGLE_TRANSF_DEF_SETTER(rot(const T& x, const T& y, const T& z),
    _rot.x = x;
    _rot.y = y;
    _rot.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(rot(const vec<2, T>& rot_xy, const T& z),
    _rot.x = rot_xy.x;
    _rot.y = rot_xy.y;
    _rot.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(rot(const T& x, const vec<2, T>& rot_yz),
    _rot.x = x;
    _rot.y = rot_yz.x;
    _rot.z = rot_yz.y;
  );
  SHOGLE_TRANSF_DEF_SETTER(rot(const complex<T>& rot_xy, const T& z),
    _rot.x = rot_xy.real();
    _rot.y = rot_xy.imag();
    _rot.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(rot(const T& x, const complex<T>& rot_yz),
    _rot.x = x;
    _rot.y = rot_yz.real();
    _rot.z = rot_yz.imag();
  );
  SHOGLE_TRANSF_DEF_SETTER(pitch(const T& pitch),
    _rot.x = pitch;
  );
  SHOGLE_TRANSF_DEF_SETTER(yaw(const T& yaw),
    _rot.y = yaw;
  );
  SHOGLE_TRANSF_DEF_SETTER(roll(const T& roll),
    _rot.z = roll;
  );

public:
  [[nodiscard]] vec<3, T> rot() const noexcept { return _rot; }
  [[nodiscard]] qua<T> qrot() const noexcept { return eulerquat(_rot); }
  [[nodiscard]] T pitch() const noexcept { return _rot.x; }
  [[nodiscard]] T yaw() const noexcept { return _rot.y; }
  [[nodiscard]] T roll() const noexcept { return _rot.z; }

protected:
  vec<3, T> _rot;
};

template<typename T, typename Derived>
class transform_rot<T, Derived, false> {
public:
  explicit transform_rot(const vec<3, T>& rot) noexcept :
    _rot{eulerquat(rot)} {}
  explicit transform_rot(const qua<T>& rot = {T{1}, T{0}, T{0}, T{0}}) noexcept :
    _rot{rot} {}

public:
  SHOGLE_TRANSF_DEF_SETTER(rot(const qua<T>& rot),
    _rot = rot;
  );
  SHOGLE_TRANSF_DEF_SETTER(rot(const vec<3, T>& rot),
    _rot = eulerquat(rot);
  );
  SHOGLE_TRANSF_DEF_SETTER(rot(const T& x, const T& y, const T& z, const T& w),
    _rot.x = x;
    _rot.y = y;
    _rot.z = z;
    _rot.w = w;
  );
  SHOGLE_TRANSF_DEF_SETTER(rot(const T& ang, const vec<3, T>& axis),
    _rot = axisquat(ang, axis);
  );
  SHOGLE_TRANSF_DEF_SETTER(rot_x(const T& x),
    _rot.x = x;
  );
  SHOGLE_TRANSF_DEF_SETTER(rot_y(const T& y),
    _rot.y = y;
  );
  SHOGLE_TRANSF_DEF_SETTER(rot_z(const T& z),
    _rot.z = z;
  );
  SHOGLE_TRANSF_DEF_SETTER(rot_w(const T& w),
    _rot.w = w;
  );

public:
  [[nodiscard]] qua<T> rot() const noexcept { return _rot; }
  [[nodiscard]] vec<3, T> erot() const noexcept { return eulerquat(_rot); }
  [[nodiscard]] T rot_x() const noexcept { return _rot.x; }
  [[nodiscard]] T rot_y() const noexcept { return _rot.y; }
  [[nodiscard]] T rot_z() const noexcept { return _rot.z; }
  [[nodiscard]] T rot_w() const noexcept { return _rot.w; }
  [[nodiscard]] vec<3, T> rot_vec() const noexcept { return vec<3, T>{_rot.x, _rot.y, _rot.z}; }

protected:
  qua<T> _rot;
};

template<typename T, typename Derived, uint32 dim, bool use_euler>
class camera_base;

template<typename T, typename Derived, bool use_euler>
class camera_base<T, Derived, 2, use_euler> {

protected:
  vec<2, T> _viewport;
  vec<2, T> _origin;
  vec<2, T> _pos;
  vec<2, T> _zoom;
  T _rot;
  T _znear;
  T _zfar;
};

template<typename T, typename Derived>
class camera_base<T, Derived, 3, true> {
public:
  camera_base(const vec<2, T> viewport,
              const T& znear       = T{0.1},
              const T& zfar        = T{100},
              const T& fov         = {glm::pi<T>()},
              const vec<3, T>& pos = {T{0}, T{0}, T{0}},
              const vec<3, T>& dir = {T{0}, T{0}, T{-1}},
              const vec<3, T>& up  = {T{0}, T{1}, T{0}}) noexcept :
    _pos{pos}, _dir{dir}, _viewport{viewport}, _up{up},
    _znear{znear}, _zfar{zfar}, _fov{fov} {}

protected:
  vec<3, T> _pos;
  vec<3, T> _dir;
  vec<2, T> _viewport;
  vec<3, T> _up;
  T _znear;
  T _zfar;
  T _fov;
};

template<typename T, typename Derived>
class camera_base<T, Derived, 3, false> {

protected:
  qua<T> _rot;
  vec<2, T> _viewport;
  vec<3, T> _pos;
  T _znear;
  T _zfar;
  T _fov;
};

} // namespace impl

enum class transf_flag {
  none              = 0,
  dirty             = 1<<0,
  auto_update       = 1<<1,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(transf_flag);
NTF_DECLARE_TAG_TYPE(transf_raw_mat);

enum class camera_flag {
  none              = 0,
  dirty_view        = 1<<0,
  dirty_proj        = 1<<1,
  auto_update       = 1<<2,
  use_perspective   = 1<<3,
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(camera_flag);
NTF_DECLARE_TAG_TYPE(camera_raw_mat);

template<typename T, uint32 dim>
struct trs_transform {
  mat<4, 4, T> operator()(const vec<dim, T>& pos,
                          const vec<dim, T>& scale,
                          const vec<dim, T>& pivot,
                          const vec<dim, T>& offset,
                          const vec<3, T>& rot) noexcept {
    return build_trs_matrix(pos, scale, pivot, offset, rot);
  }
  mat<4, 4, T> operator()(const vec<dim, T>& pos,
                          const vec<dim, T>& scale,
                          const vec<dim, T>& pivot,
                          const vec<dim, T>& offset,
                          const qua<T>& rot) noexcept {
    return build_trs_matrix(pos, scale, pivot, offset, rot);
  }
  mat<4, 4, T> operator()(const vec<dim, T>& pos,
                          const vec<dim, T>& scale,
                          const vec<dim, T>& pivot,
                          const vec<dim, T>& offset,
                          const T& roll) noexcept {
    return build_trs_matrix(pos, scale, pivot, offset, roll, vec<3, T>{T{0}, T{0}, T{1}});
  }
};

template<typename T, uint32 dim>
struct lookat_transform {
  constexpr mat<4, 4, T> operator()() noexcept {

  }
};

template<typename T, typename Transform, uint32 dim, bool use_euler>
class basic_transform final :
  public impl::transform_dim<T, basic_transform<T, Transform, dim, use_euler>, dim>,
  public impl::transform_rot<T, basic_transform<T, Transform, dim, use_euler>, use_euler> {
public:
  using value_type = T;
  using transform_type = Transform;
  static constexpr uint32 dimensions = dim;
  static constexpr bool uses_euler_angles = use_euler;

  static_assert(dim == 2 || dim == 3, "Invalid transform dimensions");

public:
  explicit basic_transform(transf_flag flags = transf_flag::dirty,
                           const Transform& transf = {})
  noexcept(std::is_nothrow_copy_constructible_v<Transform> && noexcept(_mark_dirty())) :
    impl::transform_dim<T, basic_transform<T, Transform, dim, use_euler>, dim>{},
    impl::transform_rot<T, basic_transform<T, Transform, dim, use_euler>, use_euler>{},
    _local_mat{T{1}}, _gen_mat{transf}, _flags{flags} { _mark_dirty(); }

  explicit basic_transform(const vec<dim, T>& pos,
                           const vec<dim, T>& scale,
                           const vec<dim, T>& pivot,
                           const vec<dim, T>& offset,
                           const vec<dim, T>& rot,
                           transf_flag flags = transf_flag::dirty,
                           const Transform& transf = {})
  noexcept(std::is_nothrow_copy_constructible_v<Transform> && noexcept(_mark_dirty())) :
    impl::transform_dim<T, basic_transform<T, Transform, dim, use_euler>, dim>
      {pos, scale, pivot, offset},
    impl::transform_rot<T, basic_transform<T, Transform, dim, use_euler>, use_euler>
      {rot},
    _local_mat{T{1}}, _gen_mat{transf}, _flags{flags} { _mark_dirty(); }

  explicit basic_transform(const vec<dim, T>& pos,
                           const vec<dim, T>& scale,
                           const vec<dim, T>& pivot,
                           const vec<dim, T>& offset,
                           const qua<T>& rot,
                           transf_flag flags = transf_flag::dirty,
                           const Transform& transf = {})
  noexcept(std::is_nothrow_copy_constructible_v<Transform> && noexcept(_mark_dirty())) :
    impl::transform_dim<T, basic_transform<T, Transform, dim, use_euler>, dim>
      {pos, scale, pivot, offset},
    impl::transform_rot<T, basic_transform<T, Transform, dim, use_euler>, use_euler>
      {rot},
    _local_mat{T{1}}, _gen_mat{transf}, _flags{flags} { _mark_dirty(); }

public:
  basic_transform& flags(transf_flag flags) & noexcept {
    _flags = flags;
    return *this;
  } 
  basic_transform&& flags(transf_flag flags) && noexcept {
    _flags = flags;
    return std::move(*this);
  }

public:
  const Transform& get_transform() const noexcept { return _gen_mat; }

  [[nodiscard]] const mat<4, 4, T>& local(transf_raw_mat_t) const& noexcept { return _local_mat; }
  [[nodiscard]] const mat<4, 4, T>& local() &
  noexcept(noexcept(force_update())) {
    if (+(_flags & transf_flag::dirty)) {
      force_update();
    }
    return _local_mat;
  }

  void force_update() &
  noexcept(noexcept(_gen_mat(this->_pos, this->_scale, this->_pivot, this->_offset, this->_rot))) {
    _local_mat = _gen_mat(this->_pos, this->_scale, this->_pivot, this->_offset, this->_rot);
    _flags &= ~transf_flag::dirty;
  }

  [[nodiscard]] const mat<4, 4, T>& world(transf_raw_mat_t) const& noexcept { return _local_mat;}
  [[nodiscard]] const mat<4, 4, T>& world() & noexcept(noexcept(local())) { return local(); }

  [[nodiscard]] transf_flag flags() const noexcept { return _flags; }

private:
  void _mark_dirty() noexcept(noexcept(force_update())) {
    _flags |= transf_flag::dirty;
    if (+(_flags & transf_flag::auto_update)) {
      force_update();
    }
  }

private:
  mat<4, 4, T> _local_mat;
  Transform _gen_mat;
  transf_flag _flags;

private:
  friend class impl::transform_dim<T, basic_transform<T, Transform, dim, use_euler>, dim>;
  friend class impl::transform_rot<T, basic_transform<T, Transform, dim, use_euler>, use_euler>;
};

template<typename T, typename Transform, uint32 dim, bool use_euler>
class basic_camera final :
  public impl::camera_base<T, basic_camera<T, Transform, dim, use_euler>, dim, use_euler> {
public:
  using value_type = T;
  using transform_type = Transform;
  static constexpr uint32 dimensions = dim;
  static constexpr bool uses_euler_angles = use_euler || dim == 2;

  static_assert(dim == 2 || dim == 3, "Invalid camera dimensions");

public:
  basic_camera& flags(camera_flag flags) & noexcept {
    _flags = flags;
    return *this;
  } 
  basic_camera&& flags(camera_flag flags) && noexcept {
    _flags = flags;
    return std::move(*this);
  }

public:
  const Transform& get_transform() const noexcept { return _gen_view; }

  [[nodiscard]] const mat<4, 4, T>& view(camera_raw_mat_t) const& noexcept { return _view_mat; }
  [[nodiscard]] const mat<4, 4, T>& view() &
  noexcept(noexcept(force_update_view())) {
    if (+(_flags & camera_flag::dirty_view)) {
      force_update_view();
    }
    return _view_mat;
  }

  [[nodiscard]] const mat<4, 4, T>& proj(camera_raw_mat_t) const& noexcept { return _proj_mat; }
  [[nodiscard]] const mat<4, 4 ,T>& proj() &
  noexcept(noexcept(force_update_proj())) {
    if (+(_flags & camera_flag::dirty_proj)) {
      force_update_proj();
    }
    return _proj_mat;
  }

  void force_update_view() & {
    // TODO: check noexcept here somehow
    if constexpr (dim == 2) { 
      _view_mat = _gen_view(this->_origin, this->_pos, this->_zoom, this->_rot);
    } else if constexpr(use_euler) {
      _view_mat = _gen_view(this->pos, this->dir, this->_up);
    } else {
      _view_mat = _gen_view(this->pos, this->rot);
    }
    _flags &= ~camera_flag::dirty_view;
  }

  void force_update_proj() & noexcept {
    if (+(_flags & camera_flag::use_perspective)) {
      _proj_mat = build_proj_matrix(this->_viewport, this->_znear, this->_zfar);
    } else {
      if constexpr (dim == 2) {
        _proj_mat = build_proj_matrix(this->_viewport, this->_znear, this->_zfar, glm::pi<T>());
      } else {
        _proj_mat = build_proj_matrix(this->_viewport, this->_znear, this->_zfar, this->_fov);
      }
    }
    _flags &= ~camera_flag::dirty_proj;
  }

  void force_update() &
  noexcept(noexcept(force_update_proj()) && noexcept(force_update_view())){
    force_update_view();
    force_update_proj();
  }

  [[nodiscard]] camera_flag flags() const noexcept { return _flags; }

private:
  void _mark_dirty_view()
  noexcept(noexcept(force_update_view())) {
    _flags |= camera_flag::dirty_view;
    if (+(_flags & camera_flag::auto_update)) {
      force_update_view();
    }
  }

  void _mark_dirty_proj() 
  noexcept(noexcept(force_update_proj())) {
    _flags |= camera_flag::dirty_proj;
    if (+(_flags & camera_flag::auto_update)) {
      force_update_proj();
    }
  }

private:
  mat<4, 4, T> _view_mat;
  mat<4, 4, T> _proj_mat;
  Transform _gen_view;
  camera_flag _flags;

private:
  friend class impl::camera_base<T, basic_camera<T, Transform, dim, use_euler>, dim, use_euler>;
};

template<typename T, typename Transform = trs_transform<T, 2>, bool use_euler = true>
using transform2d =
  basic_transform<T, Transform, 2, use_euler>;

template<typename T, typename Transform = trs_transform<T, 3>, bool use_euler = false>
using transform3d =
  basic_transform<T, Transform, 3, use_euler>;

template<typename T, typename CamView = lookat_transform<T, 2>>
using camera2d =
  basic_camera<T, CamView, 2, true>;

template<typename T, typename CamView = lookat_transform<T, 2>, bool use_euler = true>
using camera3d =
  basic_camera<T, CamView, 3, use_euler>;


// template<std::size_t dim>
// scene_graph<dim>::~scene_graph() noexcept {
//   for (auto* child : _children) {
//     child->_parent = nullptr;
//   }
// }
//
// template<std::size_t dim>
// auto scene_graph<dim>::add_child(scene_graph* child) & -> scene_graph& {
//   _children.push_back(child);
//   child->_parent = this;
//   child->_dirty = true; // force child to update
//   return *this;
// }
//
// template<std::size_t dim>
// auto scene_graph<dim>::add_child(scene_graph* child) && -> scene_graph&& {
//   _children.push_back(child);
//   child->_parent = this;
//   child->_dirty = true; // force child to update
//   return std::move(*this);
// }
//
// template<std::size_t dim>
// void scene_graph<dim>::force_update() & {
//   if (_parent) {
//     this->_mat = _parent->mat()*this->build_matrix(this->_pos, this->_scale, this->_rot);
//   } else {
//     this->_mat = this->build_matrix(this->_pos, this->_scale, this->_rot);
//   }
//   for (auto* child : _children) {
//     child->_dirty = true; // child may update later
//   }
//   this->_dirty = false;
// }
//
// template<std::size_t dim>
// const mat4& scene_graph<dim>::mat() & {
//   if (this->_dirty) {
//     force_update();
//   }
//   return this->_mat;
// }
//
// template<std::size_t dim>
// mat4 scene_graph<dim>::mat() && {
//   // TODO: Test this?
//   if (_parent) {
//     return _parent->mat()*this->build_matrix(this->_pos, this->_scale, this->_rot);
//   }
//   return this->build_matrix(this->_pos, this->_scale, this->_rot);
// }
//
//
// template<std::size_t dim>
// void transform<dim>::force_update() & {
//   this->_mat = this->build_matrix(this->_pos, this->_scale, this->_rot);
//   this->_dirty = false;
// }
//
// template<std::size_t dim>
// const mat4& transform<dim>::mat() & {
//   if (this->_dirty) {
//     force_update();
//   }
//   return this->_mat;
// }
//
// template<std::size_t dim>
// mat4 transform<dim>::mat() && {
//   return this->build_matrix(this->_pos, this->_scale, this->_rot);
// }

} // namespace ntf

#undef SHOGLE_TRANSF_DEF_SETTER
