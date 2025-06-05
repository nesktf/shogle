#pragma once

#include "./context.hpp"

namespace ntf::render {

enum class buffer_type : uint8 {
  vertex = 0,
  index,
  texel,
  uniform,
  shader_storage,
};

enum class buffer_flag : uint8 {
  none              = 0,
  dynamic_storage   = 1 << 0,
  read_mappable     = 1 << 1,
  write_mappable    = 1 << 2,
  rw_mappable       = (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(buffer_flag);

struct buffer_data {
  const void* data;
  size_t size;
  size_t offset;
};

template<typename T>
requires(std::is_trivially_copyable_v<T>)
buffer_data format_buffer_data(const T& data, size_t offset = 0u) {
  return {.data = std::addressof(data), .size = sizeof(T), .offset = offset};
}

struct typed_buffer_desc {
  buffer_flag flags;
  size_t size;
  weak_cptr<buffer_data> data;
};

struct buffer_desc {
  buffer_type type;
  buffer_flag flags;
  size_t size;
  weak_cptr<buffer_data> data;
};

expect<buffer_t> create_buffer(context_t ctx, const buffer_desc& desc);
void destroy_buffer(buffer_t buffer) noexcept;

expect<void> buffer_upload(buffer_t buffer, const buffer_data& data);
expect<void> buffer_upload(buffer_t buffer, size_t size, size_t offset, const void* data);
void* buffer_map(buffer_t buffer, size_t size, size_t offset);
void buffer_unmap(buffer_t buffer, void* mapped);

buffer_type buffer_get_type(buffer_t buffer);
size_t buffer_get_size(buffer_t buffer);
context_t buffer_get_ctx(buffer_t buffer);

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rbuffer_ops {
  ntfr::buffer_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::buffer_t ptr = static_cast<Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid buffer handle");
    return ptr;
  }

public:
  operator ntfr::buffer_t() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  template<typename T>
  requires(std::is_trivially_copyable_v<T>)
  ntfr::expect<void> upload(const T& data, size_t offset = 0u) const {
    return ntfr::buffer_upload(_ptr(), sizeof(T), offset, std::addressof(data));
  }

  ntfr::expect<void> upload(const ntfr::buffer_data& data) const {
    return ntfr::buffer_upload(_ptr(), data);
  }
  ntfr::expect<void> upload(size_t size, size_t offset, const void* data) const {
    return ntfr::buffer_upload(_ptr(), size, offset, data);
  }
  void* map(size_t size, size_t offset) const {
    return ntfr::buffer_map(_ptr(), size, offset);
  }
  void unmap(void* mapped) const {
    ntfr::buffer_unmap(_ptr(), mapped);
  }

  ntfr::context_view context() const {
    return {ntfr::buffer_get_ctx(_ptr())};
  }
  ntfr::buffer_type type() const {
    return ntfr::buffer_get_type(_ptr());
  }
  size_t size() const {
    return ntfr::buffer_get_size(_ptr());
  }
};

template<typename Derived>
class rbuffer_view : public rbuffer_ops<Derived> {
protected:
  rbuffer_view(ntfr::buffer_t buff) noexcept :
    _buff{buff} {}

public:
  ntfr::buffer_t get() const noexcept { return _buff; }

  bool empty() const noexcept { return _buff == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  ntfr::buffer_t _buff;
};

template<typename Derived>
class rbuffer_owning : public rbuffer_ops<Derived> {
private:
  struct deleter_t {
    void operator()(ntfr::buffer_t buff) noexcept {
      ntfr::destroy_buffer(buff);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::buffer_t>, deleter_t>;

protected:
  rbuffer_owning(ntfr::buffer_t buff) noexcept :
    _buff{buff} {}

public:
  ntfr::buffer_t get() const noexcept { return _buff.get(); }
  [[nodiscard]] ntfr::buffer_t release() noexcept { return _buff.release(); }

  bool empty() const noexcept { return _buff.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _buff;
};

} // namespace ntf::impl

namespace ntf::render {

class buffer_view : public ntf::impl::rbuffer_view<buffer_view> {
public:
  buffer_view(buffer_t buff) noexcept :
    ntf::impl::rbuffer_view<buffer_view>{buff} {}
};

class buffer : public ntf::impl::rbuffer_owning<buffer> {
public:
  explicit buffer(buffer_t buff) noexcept :
    ntf::impl::rbuffer_owning<buffer>{buff} {}

public:
  static expect<buffer> create(context_view ctx, const buffer_desc& desc) {
    return ntfr::create_buffer(ctx.get(), desc)
    .transform([](buffer_t buff) -> buffer {
      buffer d{buff};
      return buffer{buff};
    });
  }

  template<typename U, size_t N>
  static expect<buffer> create(context_view ctx, U (&arr)[N],
                               buffer_flag flags, buffer_type type, size_t offset = 0u) {
    return create(ctx, {
      .type = type,
      .flags = flags,
      .size = sizeof(arr) - offset,
      .initial_data = {
        .data = std::addressof(arr),
        .size = sizeof(arr) - offset,
        .offset = offset,
      },
    });
  }

public:
  operator buffer_view() const noexcept { return {this->get()}; }
};

template<buffer_type buff_enum>
class typed_buffer;

template<buffer_type buff_enum>
class typed_buffer_view : public ntf::impl::rbuffer_view<typed_buffer_view<buff_enum>> {
private:
  friend typed_buffer<buff_enum>;

public:
  template<buffer_type _buff_enum>
  friend typed_buffer_view<_buff_enum> to_typed(buffer_view buff) noexcept;

private:
  typed_buffer_view(buffer_t buff) noexcept :
    ntf::impl::rbuffer_view<typed_buffer_view<buff_enum>>{buff} {}

public:
  operator buffer_view() const noexcept { return {this->get()}; }
};

template<buffer_type buff_enum>
typed_buffer_view<buff_enum> to_typed(buffer_view buff) noexcept {
  buffer_t ptr = nullptr;
  if (buff.type() == buff_enum) {
    ptr = buff.get();
  }
  return typed_buffer_view<buff_enum>{ptr};
}

template<buffer_type buff_enum>
class typed_buffer : public ntf::impl::rbuffer_owning<typed_buffer<buff_enum>> {
public:
  template<buffer_type _buff_enum>
  friend typed_buffer<_buff_enum> to_typed(buffer&& buff) noexcept;

private:
  typed_buffer(buffer_t buff) noexcept :
    ntf::impl::rbuffer_owning<typed_buffer<buff_enum>>{buff} {}

public:
  static expect<typed_buffer> create(context_view ctx, const typed_buffer_desc& desc) {
    return ntfr::create_buffer(ctx.get(), {
      .type = buff_enum,
      .flags = desc.flags,
      .size = desc.size,
      .data = desc.data,
    })
    .transform([](buffer_t buff) -> typed_buffer {
      return typed_buffer{buff};
    });
  }

  template<typename U, size_t N>
  static expect<typed_buffer> create(context_view ctx, U (&arr)[N], buffer_flag flags,
                                     size_t offset = 0u) {
    return create(ctx, {
      .flags = flags,
      .size = sizeof(arr) - offset,
      .initial_data = {
        .data = std::addressof(arr),
        .size = sizeof(arr) - offset,
        .offset = offset,
      },
    });
  }

public:
  operator buffer_view() const noexcept { return {this->get()}; }
  operator typed_buffer_view<buff_enum>() const noexcept { return {this->get()}; }
};

template<buffer_type buff_enum>
typed_buffer<buff_enum> to_typed(buffer&& buff) noexcept {
  buffer_t ptr = nullptr;
  if (buff.type() == buff_enum){
    ptr = buff.release();
  }
  return typed_buffer<buff_enum>{ptr};
}

using vertex_buffer = typed_buffer<buffer_type::vertex>;
using vertex_buffer_view = typed_buffer_view<buffer_type::vertex>;

using index_buffer = typed_buffer<buffer_type::index>;
using index_buffer_view = typed_buffer_view<buffer_type::index>;

using shader_storage_buffer = typed_buffer<buffer_type::shader_storage>;
using shader_storage_buffer_view = typed_buffer_view<buffer_type::shader_storage>;

using uniform_buffer = typed_buffer<buffer_type::uniform>;
using uniform_buffer_view = typed_buffer_view<buffer_type::uniform>;

using texel_buffer = typed_buffer<buffer_type::texel>;
using texel_buffer_view = typed_buffer_view<buffer_type::texel>;

} // namespace ntf::render
