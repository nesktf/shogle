# ShOGLE
Funny C++ graphics framework that I use for my personal projects.

## Features
- Simple setup for making quick programs using 2D and 3D graphics
- Wrappers for GLFW, Imgui and OpenGL (might add some other backends in the future)
- Resource loading (texture atlases, fonts, models...)
- Some maths for physics and collisions
- Some custom stl implementations (and other extra utillities)

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

Then add the following in your project's CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(my_funny_project CXX C)
# ...
add_subdirectory("lib/shogle")
# ...
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PRIVATE lib)
target_link_libraries(${PROJECT_NAME} shogle)
```

## Examples
TODO

In the mean time you can check out my projects using this framework
- [danmaku_engine](https://github.com/nesktf/danmaku_engine) 
- [lora_gps_tracking](https://github.com/nesktf/lora_gps_tracking) 
