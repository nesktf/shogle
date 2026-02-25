#pragma once

#include <vk_mem_alloc.h>

#define VK_ASSERT(func)                                                            \
  {                                                                                \
    VkResult vkres = (func);                                                       \
    if (vkres != VK_SUCCESS) {                                                     \
      ::shogle::logger::error("[VK ERROR] {} ", ::shogle::vk_error_string(vkres)); \
      SHOGLE_ASSERT(false);                                                        \
    }                                                                              \
  }
