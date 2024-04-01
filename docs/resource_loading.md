# Resource loading system
## Definition
- The engine should load a variety of resources using a single interface
    - A good way to achieve this is using some sort of dynamic "pool" of resources
- Some examples of resources include:
    - Shaders
    - Textures
    - Models
    - Fonts
    - Sound
    - Scripts
- Resources should be loaded in a separate thread whenever possible
    - There are some resources that have to be loaded in the main thread (ex: ogl objects like shaders and textures).
    - Because of this, the loading of some (if not all) resources should be separated in two parts:
        1. Loading from disk into memory
        2. Creating the resource object in the main thread with the data
- All resources will be shared by multiple engine objects, so the pool will be the only interface that handles creating and deleting them (just to simplify things and avoid shared pointers)
    - NO other object other than the resource's source pool should hold ownership of it. All objects should access resources via const reference
- There should be some kind of global pool that handles resources which should be persistent across all the program lifetime
- The resources will have the option to define their paths and get assigned an id in some kind of register before creating the pool.

## Implementation
### Interface
- The resource pool holds a tuple of variadic containers, this way it creates only the necessary containers for each resource
```cpp
template<typename T>
using cref = std::reference_wrapper<const T>;

template<typename... TRes>
class Pool {
    using id_t = some_identifying_type;
    ...
    std::tuple<my_container<TRes>...> pool_contents;
    template<typename TResReq>
    cref<TResReq> get(id_t id) {
        return std::get<my_container<TResReq>>(pool_contents)[id];
    }
    ...
};

// Instancing
Pool<Shader, Model> my_pool;
Pool<Shader, Texture, Sound> my_other_pool;
```
- The pool accepts a list of resource paths to load in a direct or asynchronous manner. The latter needs a callback to execute after loading the list (id_t may change in the final version)
```cpp
using id_t = std::string;
my_pool.direct_load<Shader>({
    {
        .id = "my_funny_shader_id",
        .path = "/path/to/my/funny/shader.glsl"
    },
    {
        .id = "my_boring_shader_id",
        .path = "/path/to/my/boring/shader.glsl"
    }
});
my_pool.async_load<Model>({
    {
        .id = "cirno_fumo",
        .path = "/path/to/baka.obj"
    }
}, [...]{ some_loading_callback(); });
```
### Internal behaviour
- Since some objects have to be loaded in the main thread, a helper singleton is used to hold a threadpool, set a queue to load the first part of the resources in the threadpool, set another queue for loading the objects in the main thread, and execute the provided callback
```cpp
class DataLoader : public Singleton<DataLoader> {
    ...
    ThreadPool t_pool;
    std::queue<some_request_function_type> requests;
    template<typename TRes>
    TRes::data_t direct_load(path_info_t info) {
        // Construct resource data pointer...
        return my_resource_data_pointer;
    }
    template<typename TRes>
    void async_load(path_info_t info, callback_t pool_callback_on_load) {
        t_pool.enqueue([this,info,on_load,...]{
            // Construct resource data pointer...
            request.emplace([on_load,info,...](...){
                pool_callback_on_load(info, my_resource_data_pointer);
            })
        });
    }
    void do_requests() {
        while(!requests.empty()) {
            auto callback = std::move(requests.front());
            callback();
            // Some other thing
        }
    }
    ...
};
```
- Then the program main loop should call `do_requests()` 
```cpp
while(true) {
    ...
    auto& loader = DataLoader::instance();
    loader.do_requests();
    ...
}
```
- Then the loading in the main thread is handled by the resource constructor, using the provided data. unique_ptr are used to delete the residual data as soon as possible
```cpp
class Shader {
    using data_t = my_shader_data_struct;
    ...
    Shader(Shader::data_t* data) {
        // Create the resource...
    }
    ...
};

template<typename... TRes>
class Pool {
    ...
    template<typename TResReq>
    void emplace(id_t, TResReq::data_t* data_ptr) {
        // Always called in main thread
        TResReq my_resource{data_ptr};
        // Add my_resource to the corresponding container...
    }
    template<typename TResReq>
    void direct_load(my_path_list_t list) {
        for (const auto& item : list) {
            auto& loader = DataLoader::instance();
            std::unique_ptr<TResReq> ptr = loader.direct_load<TResReq>(item);
            emplace<TResReq>(ptr.get());
            // Some other thing
        }
    }
    template<typename TResReq>
    void async_load(my_path_list_t list) {
        for (const auto& item : list) {
            auto& loader = DataLoader::instance();
            loader.async_load<TResReq>(item, 
                [this,...](id_t id, std::unique_ptr<TResReq::data_t> data) {
                    // This lambda gets excecuted in do_requests();
                    emplace<TResReq>(data.get());
                   // Some other thing
                }
            );
        }
    }
    ...
};

```
### TODO
- Resource path definition
