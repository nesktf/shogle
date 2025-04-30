#pragma once

#include "./context.hpp"

namespace ntf {

class r_buffer_view {
public:
  r_buffer_view(r_buffer buffer) noexcept :
    _buffer{buffer} {}

public:
  r_expected<void> upload(size_t offset, size_t len, const void* data) const {
    return r_buffer_upload(_buffer, offset, len, data);
  }
  void upload(unchecked_t,
              size_t offset, size_t len, const void* data) const {
    r_buffer_upload(::ntf::unchecked, _buffer, offset, len, data);
  }
  r_expected<void*> map(size_t offset, size_t len) const {
    return r_buffer_map(_buffer, offset, len);
  }
  void* map(unchecked_t, size_t offset, size_t len) const {
    return r_buffer_map(::ntf::unchecked, _buffer, offset, len);
  }
  void unmap(void* ptr) const {
    r_buffer_unmap(_buffer, ptr);
  }

public:
  r_buffer handle() const { return _buffer; }
  r_context_view context() const { return r_buffer_get_ctx(_buffer); }

  r_buffer_type type() const {
    return r_buffer_get_type(_buffer);
  }
  size_t size() const {
    return r_buffer_get_size(_buffer);
  }

protected:
  r_buffer _buffer;
};

class renderer_buffer : public r_buffer_view {
private:
  struct deleter_t {
    void operator()(r_buffer buff) {
      r_destroy_buffer(buff);
    }
  };
  using uptr_type = std::unique_ptr<r_buffer_, deleter_t>;

public:
  explicit renderer_buffer(r_buffer buffer) noexcept :
    r_buffer_view{buffer},
    _handle{buffer} {}

public:
  static auto create(
    r_context_view ctx, const r_buffer_descriptor& desc
  ) noexcept -> r_expected<renderer_buffer>
  {
    return r_create_buffer(ctx.handle(), desc)
      .transform([](r_buffer buff) -> renderer_buffer {
        return renderer_buffer{buff};
      });
  }

  static auto create(
    unchecked_t,
    r_context_view ctx, const r_buffer_descriptor& desc
  ) -> renderer_buffer
  {
    return renderer_buffer{r_create_buffer(::ntf::unchecked, ctx.handle(), desc)};
  }

  template<typename U, size_t N>
  static auto create(
    r_context_view ctx, U (&arr)[N], r_buffer_type type, r_buffer_flag flags
  ) noexcept -> r_expected<renderer_buffer>
  {
    r_buffer_data bdata{
      .data = std::addressof(arr),
      .size = sizeof(arr),
      .offset = 0u,
    };
    return renderer_buffer::create(ctx, {
      .type = type,
      .flags = flags,
      .size = sizeof(arr),
      .data = &bdata,
    });
  }

  template<typename U, size_t N>
  static auto create(
    unchecked_t,
    r_context_view ctx, U (&arr)[N], r_buffer_type type, r_buffer_flag flags
  ) -> renderer_buffer
  {
    r_buffer_data bdata{
      .data = std::addressof(arr),
      .size = sizeof(arr),
      .offset = 0u,
    };
    return renderer_buffer::create(::ntf::unchecked, ctx, {
      .type = type,
      .flags = flags,
      .size = sizeof(arr),
      .data = &bdata,
    });
  }

public:
  uptr_type _handle;
};

} // namespace ntf
