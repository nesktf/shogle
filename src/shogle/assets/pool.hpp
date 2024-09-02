#pragma once

#include <shogle/shogle.hpp>

#include <shogle/core/threadpool.hpp>

namespace ntf {

using resource_id = uint32_t;

class async_data_loader {
private:
  struct data_callback_base {
    virtual ~data_callback_base() = default;
    virtual void operator()() = 0;
  };

  template<typename Data, typename Callback>
  struct data_callback : public data_callback_base {
    data_callback(Data data, Callback callback) :
      _data(std::move(data)), _callback(std::move(callback)) {}

    void operator()() override { _callback(std::move(_data)); }

    Data _data;
    Callback _callback;
  };

public:
  void do_requests();

  template<typename Data, typename Callback, typename... Args>
  void enqueue(Callback callback, Args&&... args);

private:
  std::queue<data_callback_base*> _callbacks;
  std::mutex _callback_mutex;
  thread_pool _threads;
};


namespace impl {

template<typename T, typename Data, typename Loader>
concept check_loader_return = requires(Data data, Loader loader) {
  // TODO: This thing doesn't check operator() properly
  { loader(data) } -> std::convertible_to<T>;
};

template<typename T, typename Data>
concept async_resource_data = requires(Data data) {
  typename Data::loader;
  // requires check_loader_return<T, Data, typename Data::loader>;
};

struct dummy_data_type {};
template<typename T, typename Data>
concept resource_data_type = std::same_as<Data, dummy_data_type> || async_resource_data<T, Data>;


template<typename T>
class resource_pool {
public:
  using resource_id = uint32_t;
  using resource_type = T;
  static const constexpr resource_id INVALID_ID = UINT32_MAX;

public:
  resource_pool() = default;

public:
  template<typename... Args>
  resource_id emplace(std::string name, Args&&... args);

public:
  void clear() { _resources.clear(); }

  void unload(resource_id id) { _reusable_slots.push(id); }
  

public:
  size_t size() const { return _resources.size(); }

  T& at(resource_id id) { return _resources.at(id); }
  const T& at(resource_id id) const { return _resources.at(id); }

  T& operator[](resource_id id) { return _resources[id]; }
  const T& operator[](resource_id id) const { return _resources[id]; }

  bool has(resource_id id) { return _resources.size() > id; }


  resource_id id(std::string_view name) const;

  T& at(std::string_view name) { return operator[](id(name)); }
  const T& at(std::string_view name) const { return operator[](id(name)); }

  T& operator[](std::string_view name) { return at(name); }
  const T& operator[](std::string_view name) const { return at(name); }

  bool has(std::string_view name) const { return id(name) != INVALID_ID; }

protected:
  std::vector<std::pair<std::string, T>> _resources;
  std::queue<resource_id> _reusable_slots;
};

template<typename T, typename Data>
class async_pool : public resource_pool<T> {
public:
  using resource_data = Data;
  using resource_loader = typename Data::loader;

private:
  using callback_fun = std::function<void(typename async_pool::resource_id)>;

public:
  template<typename... Args>
  void enqueue(std::string name, async_data_loader& loader, callback_fun callback, Args&&... args);
};

} // namespace impl


template<typename T, typename Data = impl::dummy_data_type>
requires(impl::resource_data_type<T, Data>)
using resource_pool = 
  std::conditional_t<impl::async_resource_data<T, Data>, impl::async_pool<T, Data>, impl::resource_pool<T>>;

} // namespace ntf

#ifndef SHOGLE_ASSETS_POOL_INL
#include <shogle/assets/pool.inl>
#endif
