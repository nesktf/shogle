#pragma once

#include <shogle/core/types.hpp>
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

public:
  resource_pool() = default;

public:
  template<typename... Args>
  resource_id emplace(std::string name, Args&&... args);

public:
  void clear() { _resources.clear(); _resource_names.clear(); }

public:
  resource_id id(std::string_view name) const { return _resource_names.at(name.data()); }

  T& at(resource_id id) { return _resources[id-1]; }
  const T& at(resource_id id) const { return _resources[id-1]; }

  T& at(std::string_view name) { return at(id(name)); }
  const T& at(std::string_view name) const { return at(id(name)); }

protected:
  strmap<resource_id> _resource_names;
  std::vector<T> _resources;
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


template<typename Data, typename Callback, typename... Args>
void async_data_loader::enqueue(Callback callback, Args&&... args) {
  _threads.enqueue([this, callback=std::move(callback), ...args=std::forward<Args>(args)]() {
    Data data = loader(std::forward<Args>(args)...);

    std::unique_lock lock{_callback_mutex};
    _callbacks.emplace(new data_callback<Data, Callback>{std::move(data), std::move(callback)});
  });
}

inline void async_data_loader::do_requests() {
  std::unique_lock lock{_callback_mutex};
  while(!_callbacks.empty()) {
    auto callback = std::move(_callbacks.front());
    _callbacks.pop();
    (*callback)();
    delete callback;
  }
}

template<typename T>
template<typename... Args>
auto impl::resource_pool<T>::emplace(std::string name, Args&&... args) -> resource_id {
  _resources.emplace_back(std::forward<Args>(args)...);

  resource_id id = _resources.size();
  _resource_names.emplace(std::make_pair(std::move(name), id));

  return id;
}

template<typename T, typename Data>
template<typename... Args>
void impl::async_pool<T, Data>::enqueue(std::string name, async_data_loader& loader, 
                                        callback_fun callback, Args&&... args) {
  loader.enqueue<resource_data>([this, name=std::move(name), callback](resource_data data) {
    resource_loader loader;
    auto id = this->emplace(std::move(name), loader(std::move(data)));
    callback(id);
  }, std::forward<Args>(args)...);
}


template<typename T, typename Data = impl::dummy_data_type>
requires(impl::resource_data_type<T, Data>)
using resource_pool = 
  std::conditional_t<impl::async_resource_data<T, Data>, impl::async_pool<T, Data>, impl::resource_pool<T>>;

} // namespace ntf
