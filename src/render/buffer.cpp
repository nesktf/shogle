#include "./internal/common.hpp"

namespace ntf {

r_buffer_::r_buffer_(r_context ctx_, r_platform_buffer handle_, const rp_buff_desc& desc) noexcept:
  rp_res_node<r_buffer_>{ctx_},
  handle{handle_},
  type{desc.type}, flags{desc.flags}, size{desc.size} {}

r_buffer_::~r_buffer_() noexcept {}

static rp_buff_desc transform_desc(const r_buffer_descriptor& desc) {
  return {
    .type = desc.type,
    .flags = desc.flags,
    .size = desc.size,
    .initial_data = desc.data,
  };
}

static r_expected<void> validate_desc(const r_buffer_descriptor& desc) {
  if (desc.data) {
    RET_ERROR_IF(!desc.data->data, "Invalid buffer data");

    RET_ERROR_IF(desc.data->size+desc.data->offset > desc.size, "Invalid buffer data offset");
  } else {
    RET_ERROR_IF(!+(desc.flags & r_buffer_flag::dynamic_storage),
                 "Can't create non dynamic buffer with no data");
  }

  return {};
}

r_expected<r_buffer> r_create_buffer(r_context ctx, const r_buffer_descriptor& desc) {
  RET_ERROR_IF(!ctx, "Invalid context handle");
  auto& alloc = ctx->alloc();
  try {
  return validate_desc(desc)
    .transform([&]() -> rp_buff_desc { return transform_desc(desc); })
    .and_then([&](rp_buff_desc&& buff_desc) -> r_expected<r_buffer> {
      r_platform_buffer handle = ctx->renderer().create_buffer(buff_desc);
      RET_ERROR_IF(!handle, "Failed to create buffer");

      auto* buff = alloc.allocate_uninited<r_buffer_>(1u);
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

void r_destroy_buffer(r_buffer buffer) noexcept {
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

static void upload_buffer_data(r_buffer buffer, size_t offset, size_t len, const void* data) {
  NTF_ASSERT(buffer);
  NTF_ASSERT(data);
  rp_buff_data desc;
  desc.data = data;
  desc.len = len;
  desc.offset = offset;
  buffer->ctx->renderer().update_buffer(buffer->handle, desc);
}

r_expected<void> r_buffer_upload(r_buffer buffer, size_t offset, size_t len, const void* data) {
  RET_ERROR_IF(!buffer, "Invalid buffer handle");

  RET_ERROR_IF(!data, "Invalid buffer data");

  RET_ERROR_IF(!+(buffer->flags & r_buffer_flag::dynamic_storage),
               "Can't update non dynamic buffer");

  RET_ERROR_IF(len+offset > buffer->size, "Invalid buffer data offset");

  try {
    upload_buffer_data(buffer, offset, len, data);
  }
  RET_ERROR_CATCH("Failed to update buffer");

  return {};
}

void* map_buffer(r_buffer buffer, size_t offset, size_t len) {
  NTF_ASSERT(buffer);
  rp_buff_mapping mapping;
  mapping.len = len;
  mapping.offset = offset;
  return buffer->ctx->renderer().map_buffer(buffer->handle, mapping);
}

r_expected<void*> r_buffer_map(r_buffer buffer, size_t offset, size_t len) {
  RET_ERROR_IF(!buffer, "Invalid buffer handle");

  RET_ERROR_IF(!+(buffer->flags & r_buffer_flag::read_mappable) ||
               !+(buffer->flags & r_buffer_flag::write_mappable),
               "Non mappable buffer");

  RET_ERROR_IF(offset+len > buffer->size,
               "Invalid mapping size");

  void* ptr = nullptr;
  try {
    ptr = map_buffer(buffer, offset, len);
    RET_ERROR_IF(!ptr, "Failed to map buffer");
  }
  RET_ERROR_CATCH("Failed to map buffer");

  return ptr;
}

void r_buffer_unmap(r_buffer buffer, void* mapped) {
  if (!buffer || !mapped) {
    return;
  }
  buffer->ctx->renderer().unmap_buffer(buffer->handle, mapped);
}

r_buffer_type r_buffer_get_type(r_buffer buffer) {
  NTF_ASSERT(buffer);
  return buffer->type;
}

size_t r_buffer_get_size(r_buffer buffer) {
  NTF_ASSERT(buffer);
  return buffer->size;
}

r_context r_buffer_get_ctx(r_buffer buffer) {
  NTF_ASSERT(buffer);
  return buffer->ctx;
}

r_platform_buffer r_buffer_get_handle(r_buffer buffer) {
  NTF_ASSERT(buffer);
  return buffer->handle;
}

} // namespace ntf
