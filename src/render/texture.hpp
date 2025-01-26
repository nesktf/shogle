#pragma once
#include "./context.hpp"

namespace ntf {

// TODO: Simplify view type definition?
class r_texture {
public:
  class view_type {
  public:
    view_type() noexcept :
      _ctx{}, _handle{} {}

  private:
    view_type(weak_ref<r_context> ctx, r_texture_handle handle) noexcept :
      _ctx{ctx}, _handle{handle} {}

  public:
    [[nodiscard]] r_texture_type type() const {
      NTF_ASSERT(has_value());
      return _ctx->query(_handle, r_query_type);
    }
    [[nodiscard]] r_texture_sampler sampler() const {
      NTF_ASSERT(has_value());
      return _ctx->query(_handle, r_query_sampler);
    }
    [[nodiscard]] r_texture_address addressing() const {
      NTF_ASSERT(has_value());
      return _ctx->query(_handle, r_query_addressing);
    }
    [[nodiscard]] r_texture_format format() const {
      NTF_ASSERT(has_value());
      return _ctx->query(_handle, r_query_format);
    }
    [[nodiscard]] uvec3 extent() const {
      NTF_ASSERT(has_value());
      return _ctx->query(_handle, r_query_extent);
    }
    [[nodiscard]] uint32 layers() const {
      NTF_ASSERT(has_value());
      return _ctx->query(_handle, r_query_layers);
    }
    [[nodiscard]] uint32 levels() const {
      NTF_ASSERT(has_value());
      return _ctx->query(_handle, r_query_levels);
    }
    [[nodiscard]] bool is_cubemap() const {
      NTF_ASSERT(has_value());
      return type() == r_texture_type::cubemap;
    }
    [[nodiscard]] uvec2 extent2d() const {
      auto d = extent(); return uvec2{d.x, d.y};
    }
    [[nodiscard]] bool is_array() const {
      return !is_cubemap() && layers() > 1;
    }
    [[nodiscard]] bool has_mipmaps() const {
      return levels() > 0;
    }

  public:
    [[nodiscard]] r_texture_handle handle() const { return _handle; }
    [[nodiscard]] bool has_value() const { return _ctx && _handle; }
    explicit operator bool() const { return has_value(); }
    explicit operator r_texture_handle() const { return handle(); }

  private:
    weak_ref<r_context> _ctx;
    r_texture_handle _handle;

  private:
    friend class r_texture;
  };

public:
  r_texture() noexcept :
    _ctx{}, _handle{} {}

private:
  r_texture(r_context& ctx, r_texture_handle handle) noexcept :
    _ctx{ctx}, _handle{handle} {}

public:
  static r_expected<r_texture> create(r_context& ctx, const r_texture_descriptor& desc) {
    auto tex = ctx.create_texture(desc);
    if (!tex) {
      return unexpected{std::move(tex.error())};
    }
    return r_texture{ctx, *tex};
  }

  static r_texture create(unchecked_t,r_context& ctx, const r_texture_descriptor& desc) {
    auto tex = ctx.create_texture(desc, ::ntf::unchecked);
    NTF_ASSERT(tex);
    return r_texture{ctx, tex};
  }

public:
  [[nodiscard]] r_expected<void> update(const r_texture_data& data) {
    NTF_ASSERT(has_value());
    return _ctx->update(_handle, data);
  }

public:
  [[nodiscard]] r_texture_type type() const {
    NTF_ASSERT(has_value());
    return _ctx->query(_handle, r_query_type);
  }
  [[nodiscard]] r_texture_sampler sampler() const {
    NTF_ASSERT(has_value());
    return _ctx->query(_handle, r_query_sampler);
  }
  [[nodiscard]] r_texture_address addressing() const {
    NTF_ASSERT(has_value());
    return _ctx->query(_handle, r_query_addressing);
  }
  [[nodiscard]] r_texture_format format() const {
    NTF_ASSERT(has_value());
    return _ctx->query(_handle, r_query_format);
  }
  [[nodiscard]] uvec3 extent() const {
    NTF_ASSERT(has_value());
    return _ctx->query(_handle, r_query_extent);
  }
  [[nodiscard]] uint32 layers() const {
    NTF_ASSERT(has_value());
    return _ctx->query(_handle, r_query_layers);
  }
  [[nodiscard]] uint32 levels() const {
    NTF_ASSERT(has_value());
    return _ctx->query(_handle, r_query_levels);
  }
  [[nodiscard]] bool is_cubemap() const {
    NTF_ASSERT(has_value());
    return type() == r_texture_type::cubemap;
  }
  [[nodiscard]] uvec2 extent2d() const {
    auto d = extent(); return uvec2{d.x, d.y};
  }
  [[nodiscard]] bool is_array() const {
    return !is_cubemap() && layers() > 1;
  }
  [[nodiscard]] bool has_mipmaps() const {
    return levels() > 0;
  }

public:
  [[nodiscard]] r_texture_handle handle() const { return _handle; }
  [[nodiscard]] bool has_value() const { return _ctx && _handle; }
  explicit operator bool() const { return has_value(); }
  operator view_type() const { return view_type{_ctx, _handle}; }
  explicit operator r_texture_handle() const { return handle(); }

private:
  weak_ref<r_context> _ctx;
  r_texture_handle _handle;

public:
  SHOGLE_DEFINE_RENDER_TYPE_MOVE(r_texture);
};
using r_texture_view = r_texture::view_type;

} // namespace ntf

#undef SHOGLE_DEFINE_RENDER_TYPE_MOVE
