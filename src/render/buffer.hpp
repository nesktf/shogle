#pragma once

#include "./context.hpp"

namespace ntf {

// TODO: Simplify view type definition?
class r_buffer {
public:
  class view_type {
  public:
    view_type() noexcept :
      _ctx{}, _handle{} {}

  private:
    view_type(weak_ref<r_context> ctx, r_buffer_handle handle) noexcept :
      _ctx{ctx}, _handle{handle} {}

  public:
    [[nodiscard]] r_buffer_type type() const {
      NTF_ASSERT(has_value());
      return _ctx->query(_handle, r_query_type);
    }

    [[nodiscard]] size_t size() const {
      NTF_ASSERT(has_value());
      return _ctx->query(_handle, r_query_size);
    }

  public:
    [[nodiscard]] r_buffer_handle handle() const { return _handle; }
    [[nodiscard]] bool has_value() const { return _ctx && _handle; }
    explicit operator bool() const { return has_value(); }
    explicit operator r_buffer_handle() const { return handle(); }

  private:
    weak_ref<r_context> _ctx;
    r_buffer_handle _handle;

  private:
    friend class r_buffer;
  };

public:
  r_buffer() noexcept :
    _ctx{}, _handle{} {}

private:
  r_buffer(r_context& ctx, r_buffer_handle handle) noexcept :
    _ctx{ctx}, _handle{handle} {}

public:
  static r_expected<r_buffer> create(r_context& ctx, const r_buffer_descriptor& desc) {
    auto buff = ctx.create_buffer(desc);
    if (!buff) {
      return unexpected{std::move(buff.error())};
    }
    return r_buffer{ctx, *buff};
  }

  static r_buffer create(unchecked_t, r_context& ctx, const r_buffer_descriptor& desc){
    r_buffer_handle buff = ctx.create_buffer(desc, ::ntf::unchecked);
    NTF_ASSERT(buff);
    return r_buffer{ctx, buff};
  }

public:
  [[nodiscard]] r_expected<void> update(const r_buffer_data& desc) {
    NTF_ASSERT(has_value());
    return _ctx->update(_handle, desc);
  }

public:
  [[nodiscard]] r_buffer_type type() const {
    NTF_ASSERT(has_value());
    return _ctx->query(_handle, r_query_type);
  }

  [[nodiscard]] size_t size() const {
    NTF_ASSERT(has_value());
    return _ctx->query(_handle, r_query_size);
  }

public:
  [[nodiscard]] r_buffer_handle handle() const { return _handle; }
  [[nodiscard]] bool has_value() const { return _ctx && _handle; }
  explicit operator bool() const { return has_value(); }
  operator view_type() const { return view_type{_ctx, _handle}; }
  explicit operator r_buffer_handle() const { return handle(); }

private:
  weak_ref<r_context> _ctx;
  r_buffer_handle _handle;

public:
  SHOGLE_DEFINE_RENDER_TYPE_MOVE(r_buffer);
};
using r_buffer_view = r_buffer::view_type;

} // namespace ntf
