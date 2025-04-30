#include "./common.hpp"

namespace ntf {

static auto transform_descriptor(
  r_context, const r_buffer_descriptor& desc
) -> r_buff_desc {
  return {
    .type = desc.type,
    .flags = desc.flags,
    .size = desc.size,
    .initial_data = desc.data,
  };
}

static auto check_and_transform_descriptor(
  r_context ctx, const r_buffer_descriptor& desc
) -> r_expected<r_buff_desc> {
  RET_ERROR_IF(!ctx,
               "[ntf::r_create_buffer]",
               "Invalid context handle");

  if (desc.data) {
    RET_ERROR_IF(!desc.data->data,
                 "[ntf::r_create_buffer]",
                 "Invalid buffer data");

    RET_ERROR_IF(desc.data->size+desc.data->offset > desc.size,
                 "[ntf::r_create_buffer]",
                 "Invalid buffer data offset");
  } else {
    RET_ERROR_IF(!+(desc.flags & r_buffer_flag::dynamic_storage),
                 "[ntf::r_create_buffer]",
                 "Attempted to create non dynamic buffer with no data");
  }

  return transform_descriptor(ctx, desc);
}

r_expected<r_buffer> r_create_buffer(r_context ctx, const r_buffer_descriptor& desc) {
  return check_and_transform_descriptor(ctx, desc)
  .and_then([ctx](r_buff_desc&& buff_desc) -> r_expected<r_buffer> {
    r_platform_buffer handle;
    try {
      handle = ctx->platform->create_buffer(buff_desc);
      RET_ERROR_IF(!handle,
                   "[ntf::r_create_buffer]",
                   "Failed to create buffer");
    }
    RET_ERROR_CATCH("[ntf::r_create_buffer]",
                    "Failed to create buffer");

    [[maybe_unused]] auto [it, emplaced] = ctx->buffers.try_emplace(
      handle, ctx, handle, std::move(buff_desc)
    );
    NTF_ASSERT(emplaced);
    return &it->second;
  });
}

r_buffer r_create_buffer(unchecked_t, r_context ctx, const r_buffer_descriptor& desc) {
  NTF_ASSERT(ctx);
  auto pdesc = transform_descriptor(ctx, desc);
  auto handle = ctx->platform->create_buffer(pdesc);
  NTF_ASSERT(handle);

  [[maybe_unused]] auto [it, emplaced] = ctx->buffers.try_emplace(
    handle, ctx, handle, std::move(pdesc)
  );
  NTF_ASSERT(emplaced);

  return &it->second;
}

void r_destroy_buffer(r_buffer buffer) {
  if (!buffer) {
    return;
  }

  const auto handle = buffer->handle;
  auto* ctx = buffer->ctx;
  auto it = ctx->buffers.find(handle);
  if (it == ctx->buffers.end()) {
    return;
  }

  ctx->platform->destroy_buffer(handle);
  ctx->buffers.erase(it);
}

static void upload_buffer_data(
  r_buffer buffer, size_t offset, size_t len, const void* data
) {
  NTF_ASSERT(buffer);
  NTF_ASSERT(data);
  r_buff_data desc;
  desc.data = data;
  desc.len = len;
  desc.offset = offset;
  buffer->ctx->platform->update_buffer(buffer->handle, desc);
}

r_expected<void> r_buffer_upload(r_buffer buffer, size_t offset, size_t len, const void* data) {
  RET_ERROR_IF(!buffer,
               "[ntf::r_buffer_upload]",
               "Invalid buffer handle");

  RET_ERROR_IF(!data,
               "[ntf::r_buffer_upload]",
               "Invalid buffer data");

  RET_ERROR_IF(!+(buffer->flags & r_buffer_flag::dynamic_storage),
               "[ntf::r_buffer_upload]",
               "Can't update non dynamic buffer");

  RET_ERROR_IF(len+offset > buffer->size,
               "[ntf::r_buffer_upload]",
               "Invalid buffer data offset");

  try {
    upload_buffer_data(buffer, offset, len, data);
  }
  RET_ERROR_CATCH("[ntf::r_buffer_upload]",
                  "Failed to update buffer");

  return {};
}

void r_buffer_upload(unchecked_t, r_buffer buffer, size_t offset, size_t len,
                     const void* data) {
  upload_buffer_data(buffer, offset, len, data);
}

void* map_buffer(r_buffer buffer, size_t offset, size_t len) {
  NTF_ASSERT(buffer);
  r_buff_mapping mapping;
  mapping.len = len;
  mapping.offset = offset;
  return buffer->ctx->platform->map_buffer(buffer->handle, mapping);
}

r_expected<void*> r_buffer_map(r_buffer buffer, size_t offset, size_t len) {
  RET_ERROR_IF(!buffer,
               "[ntf::r_buffer_map]",
               "Invalid buffer handle");

  RET_ERROR_IF(
    !+(buffer->flags & r_buffer_flag::read_mappable) ||
    !+(buffer->flags & r_buffer_flag::write_mappable),
   "[ntf::r_buffer_map]",
    "Non mappable buffer"
  );
  RET_ERROR_IF(offset+len > buffer->size,
               "[ntf::r_buffer_map]",
               "Invalid mapping size");
  void* ptr = nullptr;
  try {
    ptr = map_buffer(buffer, offset, len);
  }
  RET_ERROR_CATCH("[ntf::r_buffer_map]",
                  "Failed to map buffer");
  RET_ERROR_IF(!ptr,
               "[ntf::r_buffer_map]",
               "Failed to map buffer");
  return ptr;
}

void* r_buffer_map(unchecked_t, r_buffer buffer, size_t offset, size_t len) {
  return map_buffer(buffer, offset, len);
}

void r_buffer_unmap(r_buffer buffer, void* mapped) {
  if (!buffer || !mapped) {
    return;
  }
  buffer->ctx->platform->unmap_buffer(buffer->handle, mapped);
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
