#pragma once

#include <shogle/render/context.hpp>
#include <shogle/render/texture.hpp>
#include <shogle/render/buffer.hpp>
#include <shogle/render/framebuffer.hpp>
#include <shogle/render/pipeline.hpp>

#include <ntfstl/ptr.hpp>
#include <ntfstl/allocator.hpp>
#include <ntfstl/unique_array.hpp>
#include <ntfstl/hashmap.hpp>

#include <deque>
#include <queue>

#define RENDER_ERROR_LOG(_fmt, ...) \
  SHOGLE_LOG(error, "[{}:{}] " \
             _fmt, ::shogle::meta::parse_src_str(NTF_FILE), \
             NTF_LINE __VA_OPT__(,) __VA_ARGS__)

#define RENDER_WARN_LOG(_fmt, ...) \
  SHOGLE_LOG(warning, "[{}:{}] " \
             _fmt, ::shogle::meta::parse_src_str(NTF_FILE), \
             NTF_LINE __VA_OPT__(,) __VA_ARGS__)

#define RENDER_DBG_LOG(_fmt, ...) \
  SHOGLE_LOG(debug, "[{}:{}] " \
             _fmt, ::shogle::meta::parse_src_str(NTF_FILE), \
             NTF_LINE __VA_OPT__(,) __VA_ARGS__)

#define RENDER_VRB_LOG(_fmt, ...) \
  SHOGLE_LOG(verbose, "[{}:{}] " \
             _fmt, ::shogle::meta::parse_src_str(NTF_FILE), \
             NTF_LINE __VA_OPT__(,) __VA_ARGS__)

#define RET_ERROR(_str) \
  RENDER_ERROR_LOG(_str); \
  return ntf::unexpected{render_error{_str}}

#define RET_ERROR_IF(_cond, _str) \
  if (_cond) { RET_ERROR(_str); }

#define RET_ERROR_FMT(_alloc, _fmt, ...) \
  RENDER_ERROR_LOG(_fmt __VA_OPT__(,) __VA_ARGS__); \
  return ntf::unexpected{render_error{_alloc.fmt_arena_string(_fmt __VA_OPT__(,) __VA_ARGS__)}}

#define RET_ERROR_FMT_IF(_cond, _alloc, _fmt, ...) \
  if (_cond) { RET_ERROR_FMT(_alloc, _fmt, __VA_ARGS__); }

#define RET_ERROR_CATCH(_msg) \
  catch (render_error& err) { \
    RENDER_ERROR_LOG(_msg ": {}", err.what()); \
    return ntf::unexpected{std::move(err)}; \
  } catch (const std::bad_alloc&) { \
    RENDER_ERROR_LOG(_msg ": Allocation failed"); \
    return ntf::unexpected{render_error{"Allocation failed"}}; \
  } catch (...) { \
    RENDER_ERROR_LOG(_msg ": Caught (...)"); \
    return ntf::unexpected{render_error{"Unknown error"}}; \
  }

#define RENDER_ERROR_LOG_CATCH(_msg) \
  catch (const std::exception& ex) { \
    RENDER_ERROR_LOG(_msg ": {}", ex.what()); \
  } catch (...) { \
    RENDER_ERROR_LOG(_msg ": Caught (...)"); \
  }

namespace shogle {

class ctx_alloc {
public:
  using malloc_fun = void*(*)(void* user_ptr, size_t size, size_t alignment);
  using mfree_fun  = void (*)(void* user_ptr, void* mem, size_t size);

  template<typename T> 
  struct alloc_del_t {
    alloc_del_t(void* user_ptr, mfree_fun mfree) noexcept :
      _uptr{user_ptr}, _free{mfree} {}

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(U* ptr) noexcept(std::is_nothrow_destructible_v<T>)
    {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        static_cast<T*>(ptr)->~T();
      }
      std::invoke(_free, _uptr, ptr, sizeof(T));
    }

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(ntf::uninitialized_t, U* ptr)
    noexcept(std::is_nothrow_destructible_v<T>)
    {
      std::invoke(_free, _uptr, ptr, sizeof(T));
    }

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(ntf::uninitialized_t, U* ptr, size_t n)
    noexcept(std::is_nothrow_destructible_v<T>)
    {
      std::invoke(_free, _uptr, ptr, n*sizeof(T));
    }

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(U* ptr, size_t n) noexcept(std::is_nothrow_destructible_v<T>) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        for (U* it = ptr; it < ptr+n; ++it) {
          static_cast<T*>(it)->~T();
        }
      }
      std::invoke(_free, _uptr, ptr, n*sizeof(T));
    }

  private:
    void* _uptr;
    mfree_fun _free;
  };

  template<typename T>
  struct adaptor_t {
    using value_type = T;
    using pointer = T*;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template<typename U>
    using rebind = adaptor_t<U>;

    adaptor_t(ctx_alloc& alloc) noexcept :
      _alloc{&alloc} {}

    template<typename U>
    adaptor_t(const adaptor_t<U>& other) noexcept :
      _alloc{other._alloc} {}

    adaptor_t(const adaptor_t&) noexcept = default;
    
    pointer allocate(size_t n) {
      return _alloc->allocate_uninited<T>(n);
    }

    void deallocate(pointer ptr, size_t n) noexcept {
      _alloc->deallocate(ptr, n*sizeof(T));
    }

    constexpr bool operator==(const adaptor_t&) { return true; }
    constexpr bool operator!=(const adaptor_t&) { return false; }

    ctx_alloc* _alloc;
  };

  template<typename T>
  struct arena_adaptor_t {
    using value_type = T;
    using pointer = T*;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template<typename U>
    using rebind = adaptor_t<U>;

    arena_adaptor_t(ctx_alloc& alloc) noexcept :
      _alloc{&alloc} {}

    template<typename U>
    arena_adaptor_t(const arena_adaptor_t<U>& other) noexcept :
      _alloc{other._alloc} {}

    arena_adaptor_t(const arena_adaptor_t&) noexcept = default;
    
    pointer allocate(size_t n) {
      return _alloc->arena_allocate_uninited<T>(n);
    }

    void deallocate(pointer ptr, size_t n) noexcept {
      NTF_UNUSED(ptr);
      NTF_UNUSED(n);
    }

    constexpr bool operator==(const arena_adaptor_t&) { return true; }
    constexpr bool operator!=(const arena_adaptor_t&) { return false; }

    ctx_alloc* _alloc;
  };

  // template<typename T>
  // using adaptor_t = allocator_adaptor<T, rp_alloc>;

  template<typename T>
  using uptr_t = std::unique_ptr<T, alloc_del_t<T>>;

  template<typename T>
  using uarray_t = ntf::unique_array<T, alloc_del_t<T>>;

  template<typename T>
  using vec_t = std::vector<T, adaptor_t<T>>;

  template<typename T>
  using deque_t = std::deque<T, adaptor_t<T>>;

  template<typename T>
  using queue_t = std::queue<T, deque_t<T>>;

  template<typename T>
  using string_t = std::basic_string<T, std::char_traits<T>, adaptor_t<T>>;

  // template<typename T>
  // struct string_hash {
  //   size_t operator()(const string_t<T>& str) const {
  //     std::hash<cstring_view<T>> h{};
  //     cstring_view<T> view{str};
  //     return h(view);
  //   }
  // };

  template<typename T, typename U = char>
  using string_fhashmap_t = std::unordered_map<
    string_t<U>, T,
    std::hash<cstring_view<U>>, std::equal_to<cstring_view<U>>,
    // rp_alloc::alloc_del_t<std::pair<const std::string, r_uniform_>>
    // allocator_delete<
    //   std::pair<const string_t<U>, T>,
      adaptor_t<std::pair<const string_t<U>, T>>
    // >
  >;

public:
  ctx_alloc(const ntf::malloc_funcs& funcs, ntf::linked_arena&& arena) noexcept;

public:
  static uptr_t<ctx_alloc> make_alloc(weak_ptr<const ntf::malloc_funcs> alloc, size_t arena_size);
  
public: 
  void* allocate(size_t size, size_t alignment);
  void deallocate(void* mem, size_t sz);

  template<typename T>
  T* allocate_uninited(size_t n = 1u) {
    return static_cast<T*>(allocate(n*sizeof(T), alignof(T)));
  }

  template<typename T, typename... Args>
  T* construct(Args&&... args) {
    T* obj = static_cast<T*>(allocate(sizeof(T), alignof(T)));
    if (!obj) {
      return nullptr;
    }
    std::construct_at(obj, std::forward<Args>(args)...);
    return obj;
  }

  template<typename T>
  void destroy(T* obj) {
    obj->~T();
    deallocate(obj, sizeof(T));
  }

public:
  template<typename T>
  adaptor_t<T> make_adaptor() noexcept { return adaptor_t<T>{*this}; }

  template<typename T>
  alloc_del_t<T> make_deleter() noexcept { return alloc_del_t<T>{_user_ptr, _free}; }

  template<typename T>
  uptr_t<T> wrap_unique(T* ptr) noexcept {
    return uptr_t<T>{ptr, alloc_del_t<T>{_user_ptr, _free}};
  }

  template<typename T, typename... Args>
  uptr_t<T> make_unique(Args&&... args) {
    return uptr_t<T>{construct<T>(std::forward<Args>(args)...), alloc_del_t<T>{_user_ptr, _free}};
  }

  template<typename T>
  uarray_t<T> wrap_array(T* ptr, size_t count) {
    return uarray_t<T>{ptr, count, alloc_del_t<T>{_user_ptr, _free}};
  }

  template<typename T>
  uarray_t<T> make_uninited_array(size_t count) {
    T* ptr = allocate_uninited<T>(count);
    return uarray_t<T>{ptr ? count : 0u, ptr, alloc_del_t<T>{_user_ptr, _free}};
  }

  template<typename T>
  vec_t<T> make_vector(size_t reserve = 0u) {
    vec_t<T> vec{make_adaptor<T>()};
    if (reserve) {
      vec.reserve(reserve);
    }
    return vec;
  }

  template<typename T = char>
  string_t<T> make_string(size_t reserve = 0u) {
    string_t<T> str{make_adaptor<T>()};
    if (reserve){
      str.reserve(reserve);
    }
    return str;
  }

  template<typename T>
  string_fhashmap_t<T> make_string_map(size_t sz) {
    string_fhashmap_t<T> map{make_adaptor<std::pair<const string_t<char>, T>>()};
    map.reserve(sz);
    return map;
  }

  template<typename T = char>
  string_t<T> vfmt_string(fmt::string_view fmt, fmt::format_args args) {
    auto adaptor = make_adaptor<T>();
    fmt::basic_memory_buffer<T, fmt::inline_buffer_size, adaptor_t<T>> buff{adaptor};
    fmt::vformat_to(std::back_inserter(buff), fmt, args);
    return string_t<T>{buff.data(), buff.size(), adaptor};
  }

  template<typename T = char, typename... Args>
  string_t<T> fmt_string(fmt::string_view fmt, Args&&... args) {
    return vfmt_string(fmt, fmt::make_format_args(std::forward<Args>(args)...));
  }

public:
  void arena_clear();
  void* arena_allocate(size_t size, size_t alignment);

  template<typename T, typename... Args>
  T* arena_construct(Args&&... args) {
    return _arena.construct<T>(std::forward<Args>(args)...);
  }

  template<typename T>
  T* arena_allocate_uninited(size_t n = 1u) {
    return static_cast<T*>(arena_allocate(n*sizeof(T), alignof(T)));
  }

public:
  template<typename T>
  span<T> arena_span(size_t count) {
    return span<T>{arena_allocate_uninited<T>(count), count};
  }

  template<typename T = char>
  cstring_view<T> vfmt_arena_string(fmt::string_view fmt, fmt::format_args args) {
    arena_adaptor_t<T> adaptor{*this};
    fmt::basic_memory_buffer<T, 0u, arena_adaptor_t<T>> buff{adaptor};
    fmt::vformat_to(std::back_inserter(buff), fmt, args);
    return cstring_view<T>{buff.data(), buff.size()};
  }

  template<typename T = char, typename... Args>
  cstring_view<T> fmt_arena_string(fmt::string_view fmt, Args&&... args) {
    return vfmt_arena_string(fmt, fmt::make_format_args(std::forward<Args>(args)...));
  }

private:
  ntf::linked_arena _arena;
  void* _user_ptr;
  malloc_fun _malloc;
  mfree_fun _free;
};

} // namespace shogle
