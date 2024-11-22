#pragma once

#include "./assets.hpp"
#include "../stl/threadpool.hpp"

#include <optional>

namespace ntf {

using resource_handle = uint32_t;
constexpr resource_handle resource_tombstone = UINT32_MAX;

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
  using resource_type = T;
  using handle = ::ntf::resource_handle;

  using iterator = std::vector<std::pair<std::string, T>>::iterator;
  using const_iterator = std::vector<std::pair<std::string, T>>::const_iterator;

public:
  resource_pool() = default;

public:
  template<typename... Args>
  handle emplace(std::string name, Args&&... args);

public:
  void clear() { _resources.clear(); }
  void unload(handle id) { _reusable_slots.push(id); }

public:
  T& at(handle res);
  T& operator[](handle res);
  const T& at(handle res) const;
  const T& operator[](handle res) const;

  std::optional<handle> find(std::string_view name) const;

  size_t size() const { return _resources.size(); }
  size_t queue_size() const { return _reusable_slots.size(); }

  iterator begin() { return _resources.begin(); }
  const_iterator begin() const { return _resources.begin(); }
  const_iterator cbegin() const { return _resources.cbegin(); }

  iterator end() { return _resources.end(); }
  const_iterator end() const { return _resources.end();}
  const_iterator cend() const { return _resources.cend(); }

protected:
  std::vector<std::pair<std::string, T>> _resources;
  std::queue<handle> _reusable_slots;
};

template<typename T, typename Data>
class async_pool : public resource_pool<T> {
public:
  using resource_data = Data;
  using resource_loader = typename Data::loader;

private:
  using callback_fun = std::function<void(typename async_pool::handle, std::string)>;

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
#include "./pool.inl"
#endif
