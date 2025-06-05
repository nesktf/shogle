#include "./internal/platform.hpp"

namespace ntf::render {

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

static expect<void> validate_desc(const buffer_desc& desc) {
  if (desc.data) {
    RET_ERROR_IF(!desc.data->data, "Invalid buffer data");
    RET_ERROR_IF(desc.data->size+desc.data->offset > desc.size, "Invalid buffer data offset");
  } else {
    RET_ERROR_IF(!+(desc.flags & buffer_flag::dynamic_storage),
                 "Can't create non dynamic buffer with no data");
  }

  return {};
}

expect<buffer_t> create_buffer(context_t ctx, const buffer_desc& desc) {
  RET_ERROR_IF(!ctx, "Invalid context handle");
  auto& alloc = ctx->alloc();
  try {
    return validate_desc(desc)
    .transform([&]() -> ctx_buff_desc { return transform_desc(desc); })
    .and_then([&](ctx_buff_desc&& buff_desc) -> expect<buffer_t> {
      ctx_buff handle = CTX_HANDLE_TOMB;
      const auto ret = ctx->renderer().create_buffer(handle, buff_desc);
      RET_ERROR_IF(ret != CTX_BUFF_STATUS_OK, "Failed to create buffer");

      auto* buff = alloc.allocate_uninited<buffer_t_>(1u);
      if (!buff) {
        ctx->renderer().destroy_buffer(handle);
        RET_ERROR("Failed to allocate buffer");
      }
      std::construct_at(buff,
                        ctx, handle, buff_desc);
      ctx->insert_node(buff);
      NTF_ASSERT(buff->prev == nullptr);
      SHOGLE_LOG(debug, "[ntf::r_create_buffer] Buffer created");

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

  ctx->remove_node(buffer);
  ctx->renderer().destroy_buffer(handle);
  ctx->alloc().destroy(buffer);
  SHOGLE_LOG(debug, "[ntf::r_destroy_buffer] Buffer destroyed");
}

expect<void> buffer_upload(buffer_t buffer, size_t size, size_t offset, const void* data) {
  RET_ERROR_IF(!buffer, "Invalid buffer handle");

  RET_ERROR_IF(!data, "Invalid buffer data");

  RET_ERROR_IF(!+(buffer->flags & buffer_flag::dynamic_storage),
               "Can't update non dynamic buffer");

  RET_ERROR_IF(size+offset > buffer->size, "Invalid buffer data offset");

  try {
    const auto ret = buffer->ctx->renderer().update_buffer(buffer->handle, {
      .data = data,
      .size = size,
      .offset = offset,
    });
    RET_ERROR_IF(ret != CTX_BUFF_STATUS_OK, "Failed to update buffer");
  }
  RET_ERROR_CATCH("Failed to update buffer");

  return {};
}

void* buffer_map(buffer_t buffer, size_t size, size_t offset) {
  if (!buffer) {
    RENDER_ERROR_LOG("Invalid buffer handle");
    return nullptr;
  }

  if (!+(buffer->flags & buffer_flag::read_mappable) ||
      !+(buffer->flags & buffer_flag::write_mappable)) {
    RENDER_ERROR_LOG("Non mappable buffer");
    return nullptr;
  }

  if (offset+size > buffer->size) {
    RENDER_ERROR_LOG("Invalid mapping size");
    return nullptr;
  }

  void* ptr = nullptr;
  const auto ret = !buffer->ctx->renderer().map_buffer(buffer->handle, &ptr, size, offset);
  if (ret != CTX_BUFF_STATUS_OK){
    RENDER_ERROR_LOG("Failed to map buffer");
    return nullptr;
  }

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

} // namespace ntf::render
