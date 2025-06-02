#pragma once

#include "./context.hpp"
#include <variant>

namespace ntf::render {

enum class test_buffer : uint8 {
  no_buffer = 0,
  depth16u,
  depth24u,
  depth32f,
  depth24u_stencil8u,
  depth32f_stencil8u,
};

struct framebuffer_image {
  texture_ptr texture;
  uint32 layer;
  uint32 level;
};

using framebuffer_attachment = cspan<framebuffer_image>;

struct framebuffer_descriptor {
  extent2d extent;
  uvec4 viewport;
  color4 clear_color;
  clear_flag clear_flags;
  test_buffer buffer;
  std::variant<framebuffer_attachment, image_format> attachments;
};

expect<framebuffer_ptr> create_framebuffer(context_ptr ctx, const framebuffer_descriptor& desc);
void destroy_framebuffer(framebuffer_ptr fb);

void framebuffer_set_clear_flags(framebuffer_ptr fb, clear_flag flags);
void framebuffer_set_viewport(framebuffer_ptr fb, const uvec4& vp);
void framebuffer_set_clear_color(framebuffer_ptr fb, const color4& color);

clear_flag framebuffer_get_clear_flags(framebuffer_ptr fb);
uvec4 framebuffer_get_viewport(framebuffer_ptr fb);
color4 framebuffer_get_clear_color(framebuffer_ptr fb);

framebuffer_ptr get_default_framebuffer(context_ptr ctx);
context_ptr frambuffer_get_ctx(framebuffer_ptr fb);

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rframebuffer_ops {
  ntfr::framebuffer_ptr _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::framebuffer_ptr ptr = static_cast<Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid frambuffer handle");
    return ptr;
  }

public:
  operator ntfr::framebuffer_ptr() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  void clear_flags(ntfr::clear_flag flags) const {
    ntfr::framebuffer_set_clear_flags(_ptr(), flags);
  }
  void viewport(const uvec4& vp) const {
    ntfr::framebuffer_set_viewport(_ptr(), vp);
  }
  void clear_color(const color4& color) const {
    ntfr::framebuffer_set_clear_color(_ptr(), color);
  }

  ntfr::context_view context() const {
    return {ntfr::frambuffer_get_ctx(_ptr())};
  }
  ntfr::clear_flag clear_flags() const {
    return ntfr::framebuffer_get_clear_flags(_ptr());
  }
  uvec4 viewport() const {
    return ntfr::framebuffer_get_viewport(_ptr());
  }
  color4 clear_color() const {
    return ntfr::framebuffer_get_clear_color(_ptr());
  }
};

} // namespace ntf::impl

namespace ntf::render {

class framebuffer_view : public impl::rframebuffer_ops<framebuffer_view> {
public:
  framebuffer_view(framebuffer_ptr pip) noexcept :
    _pip{pip} {}

public:
  framebuffer_ptr get() const noexcept { return _pip;}

  bool empty() const noexcept {return _pip == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  framebuffer_ptr _pip;
};

class framebuffer : public impl::rframebuffer_ops<framebuffer> {
private:
  struct deleter_t {
    void operator()(framebuffer_ptr pip) noexcept {
      ntfr::destroy_framebuffer(pip);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::framebuffer_ptr>, deleter_t>;

public:
  static framebuffer_view get_default(context_view ctx) {
    return {ntfr::get_default_framebuffer(ctx.get())};
  }

public:
  explicit framebuffer(framebuffer_ptr pip) noexcept :
    _pip{pip} {}

public:
  static expect<framebuffer> create(context_view ctx, const framebuffer_descriptor& desc){
    return ntfr::create_framebuffer(ctx.get(), desc)
    .transform([](framebuffer_ptr pip) -> framebuffer {
      return framebuffer{pip};
    });
  }

public:
  framebuffer_ptr get() const noexcept { return _pip.get(); }
  [[nodiscard]] framebuffer_ptr release() noexcept { return _pip.release(); }

  bool empty() const noexcept { return _pip.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _pip;
};

} // namespace ntf
