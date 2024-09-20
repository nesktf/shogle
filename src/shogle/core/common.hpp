#pragma once

#define NTF_NOOP (void)0

#define NTF_DECLARE_MOVE_COPY(__type) \
  ~__type() noexcept; \
  __type(__type&&) noexcept; \
  __type(const __type&) noexcept; \
  __type& operator=(__type&&) noexcept; \
  __type& operator=(const __type&) noexcept

#define NTF_DECLARE_MOVE_ONLY(__type) \
  ~__type() noexcept; \
  __type(__type&&) noexcept; \
  __type(const __type&) = delete; \
  __type& operator=(__type&&) noexcept; \
  __type& operator=(const __type&) = delete

#define NTF_DECLARE_COPY_ONLY(__type) \
  ~__type() noexcept; \
  __type(__type&&) = delete; \
  __type(const __type&) noexcept; \
  __type& operator=(__type&&) = delete; \
  __type& operator=(const __type&) noexcept

#define NTF_DECLARE_NO_MOVE_NO_COPY(__type) \
  ~__type() noexcept; \
  __type(__type&&) = delete; \
  __type(const __type&) = delete; \
  __type& operator=(__type&&) = delete; \
  __type& operator=(const __type&) = delete

#define NTF_DISABLE_MOVE(__type) \
  __type(__type&&) = delete; \
  __type(const __type&) = default; \
  __type& operator=(__type&&) = delete; \
  __type& operator=(const __type&) = default

#define NTF_DISABLE_COPY(__type) \
  __type(__type&&) = default; \
  __type(const __type&) = delete; \
  __type& operator=(__type&&) = default; \
  __type& operator=(const __type&) = delete

#define NTF_DISABLE_MOVE_COPY(__type) \
  __type(__type&&) = delete; \
  __type(const __type&) = delete; \
  __type& operator=(__type&&) = delete; \
  __type& operator=(const __type&) = delete
