# ShOGLE
Funny C++ graphics framework that I use for my personal projects.

## Features
- Wrappers for GLFW, Imgui and OpenGL (might add some other backends in the future)
- Resource loading (texture atlases, fonts, models...)
- Some maths for physics and collisions
- Some stl implementations

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
$ sudo apt install cmake libglfw3-dev libfreetype-dev libfmt-dev libglm-dev libassimp-dev
```

Then add the following in your project's CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(my_funny_project CXX C)
# ...
add_subdirectory("lib/shogle")
# ...
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} shogle)
```

## Demos
Build them using using the following commands
```sh 
$ cmake -B build -DCMAKE_BUILD_TYPE=Debug -DSHOGLE_ENABLE_IMGUI=1 -DSHOGLE_BUILD_EXAMPLES=1
$ make -C build -j4
```

You can also check out some of my other projects that are using this framework
- [danmaku_engine](https://github.com/nesktf/danmaku_engine) 
- [lora_gps_tracking](https://github.com/nesktf/lora_gps_tracking) 
