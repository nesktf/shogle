# ShOGLE
Funny C++ graphics framework that I use for my personal projects.

## Features
- Simple setup for making quick programs using 2D and 3D graphics
- Wrappers for GLFW, Imgui and OpenGL (might add some other backends in the future)
- Resource loading (texture atlases, fonts, models...)
- Some maths for physics and collisions
- Some general utillities
    - Logger
    - Arenas
    - Threadpool
    - std::expected for c++20
    - And some other nice things

## Installation
Clone the library in your libraries folder recursing submodules

```sh 
cd my_funny_project/
mkdir -p lib/
git clone https://github.com/nesktf/shogle.git ./lib/shogle --recurse-submodules
```

You will need the following dependencies
- CMake
- GLFW3
- FreeType
- libfmt
- glm
- assimp
- PkgConfig

Only tested on Debian 12 Bookworm and Arch Linux, should work fine in other distros
if you install the appropiate dependencies. Windows is not supported (for now).

```sh
sudo apt install cmake libglfw3-dev libfreetype-dev libfmt-dev libglm-dev libassimp-dev
```

Then in your project add the following in your CMakeLists.txt

```cmake
# ... 

set(LIB_INCLUDE)
set(LIB_LINK)

# ...

set(SHOGLE_LIB "lib/shogle")
add_subdirectory(${SHOGLE_LIB})
list(APPEND LIB_INCLUDE "${SHOGLE_LIB}/shogle")
list(APPEND LIB_LINK shogle)

# ...

target_include_directories(${PROJECT_NAME} PUBLIC ${LIB_INCLUDE})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} ${LIB_LINK})

```

## Examples
TODO

In the mean time you can check out my projects using this framework
- [danmaku_engine](https://github.com/nesktf/danmaku_engine) 
- [lora_gps_tracking](https://github.com/nesktf/lora_gps_tracking) 
