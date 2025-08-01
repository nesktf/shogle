#pragma once

#include <shogle/render/context.hpp>

namespace ntf::render {

struct fbo_image {
  texture_t texture;
  uint32 layer;
  uint32 level;
};

struct fbo_image_desc {
  extent2d extent;
  uvec4 viewport;
  color4 clear_color;
  clear_flag clear_flags;
  fbo_buffer test_buffer;
  cspan<fbo_image> images;
};

// struct fbo_color_desc {
//   extent2d extent;
//   uvec4 viewport;
//   color4 clear_color;
//   clear_flag clear_flags;
//   fbo_buffer test_buffer;
//   image_format color_buffer;
// };

expect<framebuffer_t> create_framebuffer(context_t ctx, const fbo_image_desc& desc);
// expect<framebuffer_t> create_framebuffer(context_t ctx, const fbo_color_desc& desc);
void destroy_framebuffer(framebuffer_t fb) noexcept;

void framebuffer_set_clear_flags(framebuffer_t fb, clear_flag flags);
void framebuffer_set_viewport(framebuffer_t fb, const uvec4& vp);
void framebuffer_set_clear_color(framebuffer_t fb, const color4& color);

clear_flag framebuffer_get_clear_flags(framebuffer_t fb);
uvec4 framebuffer_get_viewport(framebuffer_t fb);
color4 framebuffer_get_clear_color(framebuffer_t fb);

framebuffer_t get_default_framebuffer(context_t ctx);
context_t framebuffer_get_ctx(framebuffer_t fb);
ctx_handle framebuffer_get_id(framebuffer_t fb);

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rframebuffer_ops {
  ntfr::framebuffer_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::framebuffer_t ptr = static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid frambuffer handle");
    return ptr;
  }

public:
  operator ntfr::framebuffer_t() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  void clear_flags(ntfr::clear_flag flags) const {
    ntfr::framebuffer_set_clear_flags(_ptr(), flags);
  }
  void viewport(const uvec4& vp) const {
    ntfr::framebuffer_set_viewport(_ptr(), vp);
  }
  void clear_color(const ntfr::color4& color) const {
    ntfr::framebuffer_set_clear_color(_ptr(), color);
  }

  ntfr::context_view context() const {
    return {ntfr::framebuffer_get_ctx(_ptr())};
  }
  ntfr::clear_flag clear_flags() const {
    return ntfr::framebuffer_get_clear_flags(_ptr());
  }
  ntfr::ctx_handle id() const {
    return ntfr::framebuffer_get_id(_ptr());
  }
  uvec4 viewport() const {
    return ntfr::framebuffer_get_viewport(_ptr());
  }
  ntfr::color4 clear_color() const {
    return ntfr::framebuffer_get_clear_color(_ptr());
  }
};

} // namespace ntf::impl

namespace ntf::render {

class framebuffer_view : public impl::rframebuffer_ops<framebuffer_view> {
public:
  framebuffer_view() noexcept :
    _pip{nullptr} {}

  framebuffer_view(framebuffer_t pip) noexcept :
    _pip{pip} {}

public:
  framebuffer_t get() const noexcept { return _pip;}

  bool empty() const noexcept {return _pip == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  framebuffer_t _pip;
};

class framebuffer : public impl::rframebuffer_ops<framebuffer> {
private:
  struct deleter_t {
    void operator()(framebuffer_t pip) noexcept {
      ntfr::destroy_framebuffer(pip);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::framebuffer_t>, deleter_t>;

public:
  static framebuffer_view get_default(context_view ctx) {
    return {ntfr::get_default_framebuffer(ctx.get())};
  }

public:
  framebuffer() noexcept :
    _pip{nullptr} {}

  explicit framebuffer(framebuffer_t pip) noexcept :
    _pip{pip} {}

public:
  static expect<framebuffer> create(context_view ctx, const fbo_image_desc& desc){
    return ntfr::create_framebuffer(ctx.get(), desc)
    .transform([](framebuffer_t pip) -> framebuffer {
      return framebuffer{pip};
    });
  }

public:
  framebuffer_t get() const noexcept { return _pip.get(); }
  [[nodiscard]] framebuffer_t release() noexcept { return _pip.release(); }

  bool empty() const noexcept { return _pip.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _pip;
};

} // namespace ntf
