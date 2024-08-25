#define SHOGLE_ASSETS_POOL_INL
#include <shogle/assets/pool.hpp>
#undef SHOGLE_ASSETS_POOL_INL

namespace ntf {

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

  resource_id id = _resources.size()-1;
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

} // namespace ntf
