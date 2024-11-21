#define SHOGLE_ASSETS_POOL_INL
#include "./pool.hpp"
#undef SHOGLE_ASSETS_POOL_INL

namespace ntf {

template<typename Data, typename Callback, typename... Args>
void async_data_loader::enqueue(Callback callback, Args&&... args) {
  _threads.enqueue([this, callback=std::move(callback), ...args=std::forward<Args>(args)]() mutable {
    Data data{std::forward<Args>(args)...};

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
auto impl::resource_pool<T>::emplace(std::string name, Args&&... args) -> handle {
  if (_reusable_slots.size() > 0) {
    handle id = _reusable_slots.front();
    _reusable_slots.pop();
    _resources[id] = std::make_pair(std::move(name), T{std::forward<Args>(args)...});
    return id;
  }

  handle id = _resources.size();
  _resources.emplace_back(std::make_pair(std::move(name), T{std::forward<Args>(args)...}));
  return id;
}

template<typename T>
T& impl::resource_pool<T>::at(handle res) {
  assert(res < size() && "Resource handle out of range");
  return _resources[res].second;
}

template<typename T>
T& impl::resource_pool<T>::operator[](handle res) {
  assert(res < size() && "Resource handle out of range");
  return _resources[res].second;
}

template<typename T>
const T& impl::resource_pool<T>::at(handle res) const {
  assert(res < size() && "Resource handle out of range");
  return _resources[res].second;
}

template<typename T>
const T& impl::resource_pool<T>::operator[](handle res) const {
  assert(res < size() && "Resource handle out of range");
  return _resources[res].second;
}

template<typename T>
auto impl::resource_pool<T>::find(std::string_view name) const -> std::optional<handle> {
  auto it = std::find_if(_resources.begin(), _resources.end(), [name](const auto& res) {
    return res.first == name;
  });
  if (it != _resources.end()) {
    return {static_cast<handle>(std::distance(_resources.begin(), it))};
  }

  return {};
}

template<typename T, typename Data>
template<typename... Args>
void impl::async_pool<T, Data>::enqueue(std::string name, async_data_loader& loader, 
                                        callback_fun callback, Args&&... args) {
  loader.enqueue<resource_data>([this, name=std::move(name), callback=std::move(callback)](resource_data data) {
    resource_loader loader;
    auto id = this->emplace(name, loader(std::move(data)));
    callback(id, std::move(name));
  }, std::forward<Args>(args)...);
}

} // namespace ntf
