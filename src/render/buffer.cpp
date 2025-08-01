#include "./buffer.hpp"
#include "./context.hpp"

namespace shogle {

buffer_t_::buffer_t_(context_t ctx_, ctx_buff handle_, const ctx_buff_desc& desc) noexcept:
  ctx_res_node<buffer_t_>{ctx_},
  handle{handle_},
  type{desc.type}, flags{desc.flags}, size{desc.size} {}

buffer_t_::~buffer_t_() noexcept {}

static ctx_buff_desc transform_desc(const buffer_desc& desc) {
  return {
    .type = desc.type,
    .flags = desc.flags,
    .size = desc.size,
    .data = desc.data,
  };
}

static render_expect<void> validate_desc(const buffer_desc& desc) {
  if (desc.data) {
    if (!desc.data->data) {
      return {ntf::unexpect, render_error::no_data};
    }
    if (desc.data->size+desc.data->offset > desc.size) {
      return {ntf::unexpect, render_error::invalid_offset};
    }
  } else {
    if (!+(desc.flags & buffer_flag::dynamic_storage)) {
      return {ntf::unexpect, render_error::buff_not_dynamic};
    }
  }

  return {};
}

static const char* buffer_type_str(buffer_type type) {
  switch (type) {
    case buffer_type::vertex: return "VERTEX";
    case buffer_type::index: return "INDEX";
    case buffer_type::shader_storage: return "SHADER_STORAGE";
    case buffer_type::texel: return "TEXEL";
    case buffer_type::uniform: return "UNIFORM";
  }
  NTF_UNREACHABLE();
}

render_expect<buffer_t> create_buffer(context_t ctx, const buffer_desc& desc) {
  if (!ctx) {
    return {ntf::unexpect, render_error::invalid_handle};
  }
  auto& alloc = ctx->alloc();
  try {
    return validate_desc(desc)
    .transform([&]() -> ctx_buff_desc { return transform_desc(desc); })
    .and_then([&](ctx_buff_desc&& buff_desc) -> render_expect<buffer_t> {
      auto* buff = alloc.allocate_uninited<buffer_t_>();
      NTF_ASSERT(buff);

      ctx_buff handle = CTX_HANDLE_TOMB;
      const auto ret = ctx->renderer().create_buffer(handle, buff_desc);
      if (ret != render_error::no_error) {
        alloc.deallocate(buff, sizeof(buffer_t_));
        return {ntf::unexpect, ret};
      }

      NTF_ASSERT(check_handle(handle));
      std::construct_at(buff,
                        ctx, handle, buff_desc);
      ctx->insert_node(buff);
      NTF_ASSERT(buff->prev == nullptr);
      SHOGLE_LOG(verbose, "Buffer created {} [type: {}]",
                     buff->handle, buffer_type_str(desc.type));

      return buff;
    });
  } RET_ERROR_CATCH("Failed to create buffer");
}

void destroy_buffer(buffer_t buffer) noexcept {
  if (!buffer) {
    return;
  }
  const auto handle = buffer->handle;
  auto* ctx = buffer->ctx;
  SHOGLE_LOG(verbose, "Buffer destroyed ({}) [type: {}]",
                 buffer->handle, buffer_type_str(buffer->type));

  ctx->remove_node(buffer);
  ctx->renderer().destroy_buffer(handle);
  ctx->alloc().destroy(buffer);
}

render_expect<void> buffer_upload(buffer_t buffer, const buffer_data& data) {
  return buffer_upload(buffer, data.size, data.offset, data.data);
}

render_expect<void> buffer_upload(buffer_t buffer, size_t size, size_t offset, const void* data) {
  if (!buffer) {
    return {ntf::unexpect, render_error::invalid_handle};
  }

  if (!data) {
    return {ntf::unexpect, render_error::no_data};
  }

  if (!+(buffer->flags & buffer_flag::dynamic_storage)) {
    return {ntf::unexpect, render_error::buff_not_dynamic};
  }

  if (size+offset > buffer->size) {
    return {ntf::unexpect, render_error::invalid_offset};
  }

  try {
    const auto ret = buffer->ctx->renderer().update_buffer(buffer->handle, {
      .data = data,
      .size = size,
      .offset = offset,
    });
    if (ret != render_error::no_error) {
      return {ntf::unexpect, ret};
    }
  }
  RET_ERROR_CATCH("Failed to update buffer");

  return {};
}

render_expect<void*> buffer_map(buffer_t buffer, size_t size, size_t offset) {
  if (!buffer) {
    return {ntf::unexpect, render_error::invalid_handle};
  }

  const bool nonmappable = !+(buffer->flags & buffer_flag::read_mappable) ||
              !+(buffer->flags & buffer_flag::write_mappable);
  if (nonmappable) {
    return {ntf::unexpect, render_error::buff_not_mappable};
  }

  if (offset+size > buffer->size) {
    return {ntf::unexpect, render_error::invalid_offset};
  }

  void* ptr = nullptr;
  const auto ret = buffer->ctx->renderer().map_buffer(buffer->handle, &ptr, size, offset);
  if (ret != render_error::no_error){
    return {ntf::unexpect, ret};
  }
  NTF_ASSERT(ptr);
  return ptr;
}

void buffer_unmap(buffer_t buffer, void* mapped) {
  if (!buffer || !mapped) {
    return;
  }
  buffer->ctx->renderer().unmap_buffer(buffer->handle, mapped);
}

buffer_type buffer_get_type(buffer_t buffer) {
  NTF_ASSERT(buffer);
  return buffer->type;
}

size_t buffer_get_size(buffer_t buffer) {
  NTF_ASSERT(buffer);
  return buffer->size;
}

context_t buffer_get_ctx(buffer_t buffer) {
  NTF_ASSERT(buffer);
  return buffer->ctx;
}

ctx_handle buffer_get_id(buffer_t buffer) {
  NTF_ASSERT(buffer);
  return buffer->handle;
}

} // namespace shogle
