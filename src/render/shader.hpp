#pragma once

#include "./context.hpp"

namespace ntf::render {

enum class shader_type : uint8 {
  vertex = 0,
  fragment,
  geometry,
  tesselation_eval,
  tesselation_control,
  compute,
};

using typed_shader_descriptor = cspan<std::string_view>;

struct shader_descriptor {
  shader_type type;
  cspan<std::string_view> source;
};

expect<shader_ptr> create_shader(context_ptr ctx, const shader_descriptor& desc);
void destroy_shader(shader_ptr shader) noexcept;

shader_type shader_get_type(shader_ptr shader);
context_ptr shader_get_ctx(shader_ptr shader);

} // namespace ntf::rendere

namespace ntf::impl {

template<typename Derived>
class rshader_ops {
  ntfr::shader_ptr _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::shader_ptr ptr = static_cast<Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid shader handle");
    return ptr;
  }

public:
  operator ntfr::shader_ptr() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  ntfr::context_view context() const {
    return {ntfr::shader_get_ctx(_ptr())};
  }
  ntfr::shader_type type() const {
    return ntfr::shader_get_type(_ptr());
  }
};

template<typename Derived>
class rshader_view : public rshader_ops<Derived> {
protected:
  rshader_view(ntfr::shader_ptr shad) noexcept :
    _shad{shad} {}

public:
  ntfr::shader_ptr get() const noexcept { return _shad; }

  bool empty() const noexcept { return _shad == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  ntfr::shader_ptr _shad;
};

template<typename Derived>
class rshader_owning : public rshader_ops<Derived> {
private:
  struct deleter_t {
    void operator()(ntfr::shader_ptr shad) noexcept {
      ntfr::destroy_shader(shad);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::shader_ptr>, deleter_t>;

protected:
  rshader_owning(ntfr::shader_ptr shad) noexcept :
    _shad{shad} {}

public:
  ntfr::shader_ptr get() const noexcept { return _shad.get(); }
  [[nodiscard]] ntfr::shader_ptr release() noexcept { return _shad.release(); }

  bool empty() const noexcept { return _shad.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _shad;
};

} // namespace ntf::impl

namespace ntf::render {

class shader_view : public impl::rshader_view<shader_view> {
public:
  shader_view(ntfr::shader_ptr shad) noexcept :
    impl::rshader_view<shader_view>{shad} {}
};

class shader : public impl::rshader_owning<shader> {
public:
  explicit shader(ntfr::shader_ptr shad) noexcept :
    impl::rshader_owning<shader>{shad} {}
  
public:
  static expect<shader> create(context_view ctx, const shader_descriptor& desc) {
    return ntfr::create_shader(ctx.get(), desc)
    .transform([](shader_ptr shad) -> shader {
      return shader{shad};
    });
  }

public:
  operator shader_view() const noexcept { return {this->get()}; }
};

template<shader_type shad_enum>
class typed_shader;

template<shader_type shad_enum>
class typed_shader_view : public impl::rshader_view<typed_shader_view<shad_enum>> {
private:
  friend typed_shader<shad_enum>;

public:
  template<shader_type _shad_enum>
  friend typed_shader_view<_shad_enum> to_typed(shader_view shad) noexcept;

private:
  typed_shader_view(shader_ptr shad) noexcept :
    impl::rshader_view<typed_shader_view<shad_enum>>{shad} {}

public:
  operator shader_view() const noexcept { return {this->get()}; }
};

template<shader_type shad_enum>
typed_shader_view<shad_enum> to_typed(shader_view shad) noexcept {
  shader_ptr ptr = nullptr;
  if (shad.type() == shad_enum) {
    ptr = shad.get();
  }
  return typed_shader_view<shad_enum>{ptr};
}

template<shader_type shad_enum>
class typed_shader : public impl::rshader_owning<typed_shader<shad_enum>> {
public:
  template<shader_type _shad_enum>
  friend typed_shader<_shad_enum> to_typed(shader&& shad) noexcept;

public:
  typed_shader(shader_ptr shad) noexcept :
    impl::rshader_owning<typed_shader<shad_enum>>{shad} {}

public:
  static expect<typed_shader> create(context_view ctx, typed_shader_descriptor desc) {
    return ntfr::create_shader(ctx, {
      .type = shad_enum,
      .source = desc,
    })
    .transform([](shader_ptr shad) -> typed_shader {
      return typed_shader{shad};
    });
  }

public:
  operator shader_view() const noexcept { return {this->get()}; }
  operator typed_shader_view<shad_enum>() const noexcept { return {this->get()}; }
};

template<shader_type shad_enum>
typed_shader<shad_enum> to_typed(shader&& shad) noexcept {
  shader_ptr ptr = nullptr;
  if (shad.type() == shad_enum) {
    ptr = shad.release();
  }
  return typed_shader<shad_enum>{ptr};
}

using vertex_shader = typed_shader<shader_type::vertex>;
using vertex_shader_view = typed_shader_view<shader_type::vertex>;

using fragment_shader = typed_shader<shader_type::fragment>;
using fragment_shader_view = typed_shader_view<shader_type::fragment>;

using geometry_shader = typed_shader<shader_type::geometry>;
using geometry_shader_view = typed_shader_view<shader_type::geometry>;

using tess_control_shader = typed_shader<shader_type::tesselation_control>;
using tess_control_shader_view = typed_shader_view<shader_type::tesselation_control>;

using tess_eval_shader = typed_shader<shader_type::tesselation_eval>;
using tess_eval_shader_view = typed_shader_view<shader_type::tesselation_eval>;

using compute_shader = typed_shader<shader_type::compute>;
using compute_shader_view = typed_shader_view<shader_type::compute>;

} // namespace ntf
